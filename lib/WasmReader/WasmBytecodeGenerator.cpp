//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

// TODO (michhol): cleanup includes
#include "Bytecode\AsmJsByteCodeWriter.h"
#include "Bytecode\ByteCodeDumper.h"
#include "Bytecode\AsmJsByteCodeDumper.h"
#include "Language\AsmJSTypes.h"

#ifdef ENABLE_WASM

namespace Wasm
{
WasmBytecodeGenerator::WasmBytecodeGenerator(Js::ScriptContext * scriptContext, Js::Utf8SourceInfo * sourceInfo, BaseWasmReader * reader) :
    m_scriptContext(scriptContext),
    m_sourceInfo(sourceInfo),
    m_alloc(L"WasmBytecodeGen", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory),
    m_reader(reader),
    m_i32RegSlots(ReservedRegisterCount),
    m_f32RegSlots(ReservedRegisterCount),
    m_f64RegSlots(ReservedRegisterCount)
{
    m_writer.Create();

    // TODO (michhol): try to make this more accurate?
    const long astSize = 0;
    m_writer.InitData(&m_alloc, astSize);
}

WasmScript *
WasmBytecodeGenerator::GenerateWasmScript()
{
    // TODO: can this be in a better place?
    m_sourceInfo->EnsureInitialized(0);
    m_sourceInfo->GetSrcInfo()->sourceContextInfo->EnsureInitialized();

    // TODO: should scripts support multiple modules?

    m_wasmScript = Anew(&m_alloc, WasmScript);
    m_wasmScript->globalBody = Js::FunctionBody::NewFromRecycler(m_scriptContext, L"[Global WebAssembly Code]", 19, 0, 0, m_sourceInfo, m_sourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, 0, nullptr, Js::FunctionInfo::Attributes::None);

    // TODO (michhol): numbering
    m_wasmScript->globalBody->SetSourceInfo(0);
    WasmOp op;
    while ((op = m_reader->ReadFromScript()) != wnLIMIT)
    {
        switch (op)
        {
        case wnMODULE:
            m_wasmScript->module = GenerateModule();
            break;
        case wnINVOKE:
            GenerateInvoke();
            break;
        case wnASSERT:
        default:
            Assert(UNREACHED);
        }
    }
    return m_wasmScript;
}

WasmModule *
WasmBytecodeGenerator::GenerateModule()
{
    WasmOp op;
    m_module = Anew(&m_alloc, WasmModule);
    m_module->functions = Anew(&m_alloc, WasmFunctionArray, &m_alloc, 0);
    m_module->exports = Anew(&m_alloc, WasmExportDictionary, &m_alloc);

    while ((op = m_reader->ReadFromModule()) != wnLIMIT)
    {
        switch (op)
        {
        case wnFUNC:
            m_module->functions->Add(GenerateFunction());
            break;
        case wnEXPORT:
            AddExport();
            break;
            // TODO: implement below ops
        case wnGLOBAL:
        case wnTABLE:
        case wnMEMORY:
        case wnDATA:
        default:
            Assert(UNREACHED);
        }
    }

    return m_module;
}

WasmFunction *
WasmBytecodeGenerator::GenerateFunction()
{
    m_func = Anew(&m_alloc, WasmFunction);
    m_func->body = Js::FunctionBody::NewFromRecycler(m_scriptContext, L"func", 5, 0, 0, m_sourceInfo, m_sourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, 0, nullptr, Js::FunctionInfo::Attributes::None);
    // TODO (michhol): numbering
    m_func->body->SetSourceInfo(0);
    m_func->body->AllocateAsmJsFunctionInfo();
    m_func->body->SetIsAsmJsFunction(true);
    m_func->body->SetIsAsmjsMode(true);
    m_func->body->SetIsWasmFunction(true);
    m_funcInfo = m_reader->m_currentNode.func.info;
    m_nestedIfLevel = 0;
    EnregisterLocals();

    // TODO: fix these bools
    m_writer.Begin(m_func->body, &m_alloc, false, true, false);


    WasmOp op;
    while ((op = m_reader->ReadExpr()) != wnLIMIT)
    {
        EmitInfo info = EmitExpr(op);
        // REVIEW: should this be Assert or throw if false, or does this even hold?
        Assert(info.type == WasmTypes::Void);
    }

    m_writer.End();

    if (PHASE_DUMP(Js::ByteCodePhase, m_func->body))
    {
        Js::AsmJsByteCodeDumper::DumpBasic(m_func->body);
    }

    Js::AsmJsFunctionInfo * info = m_func->body->GetAsmJsFunctionInfo();
    if (m_funcInfo->GetParamCount() >= Js::Constants::InvalidArgSlot)
    {
        Js::Throw::OutOfMemory();
    }
    Js::ArgSlot paramCount = (Js::ArgSlot)m_funcInfo->GetParamCount();
    info->SetArgCount(paramCount);

    info->SetArgSizeArrayLength(max(paramCount, 3ui16));
    uint* argSizeArray = RecyclerNewArrayLeafZ(m_scriptContext->GetRecycler(), uint, paramCount);
    info->SetArgsSizesArray(argSizeArray);

    if (paramCount > 0)
    {
        info->SetArgTypeArray(RecyclerNewArrayLeaf(m_scriptContext->GetRecycler(), Js::AsmJsVarType::Which, paramCount));
    }
    Js::ArgSlot paramSize = 0;
    for (Js::ArgSlot i = 0; i < paramCount; ++i)
    {
        WasmTypes::WasmType type = m_funcInfo->GetParam(i);
        info->SetArgType(GetAsmJsVarType(type), i);
        uint16 size = 0;
        switch (type)
        {
        case WasmTypes::F32:
        case WasmTypes::I32:
            CompileAssert(sizeof(float) == sizeof(int32));
#ifdef _M_X64
            // on x64, we always alloc (at least) 8 bytes per arguments
            size = sizeof(void*);
#elif _M_IX86
            size = sizeof(int32);
#else
            Assert(UNREACHED);
#endif
            break;
        case WasmTypes::F64:
        case WasmTypes::I64:
            CompileAssert(sizeof(double) == sizeof(int64));
            size = sizeof(int64);
            break;
        default:
            Assume(UNREACHED);
        }
        argSizeArray[i] = size;
        // REVIEW: reduce number of checked adds
        paramSize = UInt16Math::Add(paramSize, size);
    }
    info->SetArgByteSize(paramSize);

    info->SetIntVarCount(m_i32RegSlots.GetVarCount());
    info->SetFloatVarCount(m_f32RegSlots.GetVarCount());
    info->SetDoubleVarCount(m_f64RegSlots.GetVarCount());

    info->SetIntTmpCount(m_i32RegSlots.GetTmpCount());
    info->SetFloatTmpCount(m_f32RegSlots.GetTmpCount());
    info->SetDoubleTmpCount(m_f64RegSlots.GetTmpCount());

    info->SetIntConstCount(ReservedRegisterCount);
    info->SetFloatVarCount(ReservedRegisterCount);
    info->SetDoubleVarCount(ReservedRegisterCount);

    info->SetReturnType(GetAsmJsReturnType());

    // Review: overflow checks? 
    info->SetIntByteOffset(ReservedRegisterCount * sizeof(Js::Var));
    info->SetFloatByteOffset(info->GetIntByteOffset() + m_i32RegSlots.GetRegisterCount() * sizeof(int32));
    info->SetDoubleByteOffset(Math::Align<int>(info->GetFloatByteOffset() + m_f32RegSlots.GetRegisterCount() * sizeof(float), sizeof(double)));

    return m_func;
}

void
WasmBytecodeGenerator::EnregisterLocals()
{
    m_locals = AnewArray(&m_alloc, WasmLocal, m_funcInfo->GetLocalCount());

    for (uint i = 0; i < m_funcInfo->GetLocalCount(); ++i)
    {
        WasmTypes::WasmType type = m_funcInfo->GetLocal(i);
        WasmRegisterSpace * regSpace = GetRegisterSpace(type);

        m_locals[i] = WasmLocal(regSpace->AcquireRegister(), type);
    }
}

EmitInfo
WasmBytecodeGenerator::EmitExpr(WasmOp op)
{
    switch (op)
    {
    case wnGETLOCAL:
        return EmitGetLocal();
    case wnSETLOCAL:
        return EmitSetLocal();
    case wnRETURN:
        return EmitReturnExpr();
    case wnCONST:
        return EmitConst();
    case wnIF:
        return EmitIfExpr();

#define WASM_KEYWORD_BIN_MATH(token, ...) \
    case wn##token: \
        return EmitBinExpr(wn##token);

#define WASM_KEYWORD_COMPARE(token, ...) \
    case wn##token: \
        return EmitCompareExpr(wn##token);

#include "WasmKeywords.h"

    default:
        Assert(UNREACHED);
    }
    return EmitInfo();
}

EmitInfo
WasmBytecodeGenerator::EmitGetLocal()
{
    if (m_funcInfo->GetLocalCount() < m_reader->m_currentNode.var.num)
    {
        throw WasmCompilationException(L"%u is not a valid local", m_reader->m_currentNode.var.num);
    }

    WasmLocal local = m_locals[m_reader->m_currentNode.var.num];

    Js::OpCodeAsmJs op = Js::OpCodeAsmJs::Nop;
    switch (local.type)
    {
    case WasmTypes::F32:
        op = Js::OpCodeAsmJs::Ld_Flt;
        break;
    case WasmTypes::F64:
        op = Js::OpCodeAsmJs::Ld_Db;
        break;
    case WasmTypes::I32:
        op = Js::OpCodeAsmJs::Ld_Int;
        break;
    default:
        Assume(UNREACHED);
    }

    WasmRegisterSpace * regSpace = GetRegisterSpace(local.type);

    Js::RegSlot tmpReg = regSpace->AcquireTmpRegister();

    m_writer.AsmReg2(op, tmpReg, local.location);

    return EmitInfo(tmpReg, local.type);
}

EmitInfo
WasmBytecodeGenerator::EmitSetLocal()
{
    if (m_funcInfo->GetLocalCount() < m_reader->m_currentNode.var.num)
    {
        throw WasmCompilationException(L"%u is not a valid local", m_reader->m_currentNode.var.num);
    }

    WasmLocal local = m_locals[m_reader->m_currentNode.var.num];

    Js::OpCodeAsmJs op = Js::OpCodeAsmJs::Nop;
    WasmRegisterSpace * regSpace = nullptr;
    switch (local.type)
    {
    case WasmTypes::F32:
        op = Js::OpCodeAsmJs::Ld_Flt;
        regSpace = &m_f32RegSlots;
        break;
    case WasmTypes::F64:
        op = Js::OpCodeAsmJs::Ld_Db;
        regSpace = &m_f64RegSlots;
        break;
    case WasmTypes::I32:
        op = Js::OpCodeAsmJs::Ld_Int;
        regSpace = &m_i32RegSlots;
        break;
    default:
        Assume(UNREACHED);
    }

    EmitInfo info = EmitExpr(m_reader->ReadExpr());

    if (info.type != local.type)
    {
        throw WasmCompilationException(L"TypeError in setlocal for %u", m_reader->m_currentNode.var.num);
    }

    m_writer.AsmReg2(op, local.location, info.location);

    regSpace->ReleaseLocation(&info);

    // REVIEW: should this produce result of setlocal? currently produces void
    return EmitInfo();
}

EmitInfo
WasmBytecodeGenerator::EmitConst()
{
    WasmRegisterSpace * regSpace = GetRegisterSpace(m_reader->m_currentNode.type);

    Js::RegSlot tmpReg = regSpace->AcquireTmpRegister();

    switch (m_reader->m_currentNode.type)
    {
    case WasmTypes::F32:
        m_writer.AsmFloat1Const1(Js::OpCodeAsmJs::Ld_FltConst, tmpReg, m_reader->m_currentNode.cnst.f32);
        break;
    case WasmTypes::F64:
        m_writer.AsmDouble1Const1(Js::OpCodeAsmJs::Ld_DbConst, tmpReg, m_reader->m_currentNode.cnst.f64);
        break;
    case WasmTypes::I32:
        m_writer.AsmInt1Const1(Js::OpCodeAsmJs::Ld_IntConst, tmpReg, m_reader->m_currentNode.cnst.i32);
        break;
    default:
        Assume(UNREACHED);
    }

    return EmitInfo(tmpReg, m_reader->m_currentNode.type);
}

EmitInfo
WasmBytecodeGenerator::EmitIfExpr()
{
    ++m_nestedIfLevel;

    if (m_nestedIfLevel == 0)
    {
        // overflow
        Js::Throw::OutOfMemory();
    }

    EmitInfo checkExpr = EmitExpr(m_reader->ReadExpr());

    if (checkExpr.type != WasmTypes::I32)
    {
        throw WasmCompilationException(L"If expression must have type i32");
    }

    // TODO: save this so I can break
    Js::ByteCodeLabel falseLabel = m_writer.DefineLabel();

    m_writer.AsmBrReg1(Js::OpCodeAsmJs::BrFalse_Int, falseLabel, checkExpr.location);

    GetRegisterSpace(checkExpr.type)->ReleaseLocation(&checkExpr);

    EmitInfo innerExpr = EmitExpr(m_reader->ReadExpr());

    if (innerExpr.type != WasmTypes::Void)
    {
        throw WasmCompilationException(L"Result of if must be void");
    }

    m_writer.MarkAsmJsLabel(falseLabel);

    Assert(m_nestedIfLevel > 0);
    --m_nestedIfLevel;

    return EmitInfo();

}

template<typename T>
Js::RegSlot
WasmBytecodeGenerator::GetConstReg(T constVal)
{
    Js::RegSlot location = m_funcInfo->GetConst(constVal);
    if (location == Js::Constants::NoRegister)
    {
        WasmRegisterSpace * regSpace = GetRegisterSpace(m_reader->m_currentNode.type);
        location = regSpace->AcquireConstRegister();
        m_funcInfo->AddConst(constVal, location);
    }
    return location;
}

EmitInfo
WasmBytecodeGenerator::EmitReturnExpr()
{
    if (m_reader->m_currentNode.opt.exists)
    {
        if (m_funcInfo->GetResultType() == WasmTypes::Void)
        {
            throw WasmCompilationException(L"Void result type cannot return expression");
        }

        EmitInfo retExprInfo = EmitExpr(m_reader->ReadExpr());

        if (m_funcInfo->GetResultType() != retExprInfo.type)
        {
            throw WasmCompilationException(L"Result type must match return type");
        }

        Js::OpCodeAsmJs retOp = Js::OpCodeAsmJs::Nop;
        switch (retExprInfo.type)
        {
        case WasmTypes::F32:
            retOp = Js::OpCodeAsmJs::Return_Flt;
            m_func->body->GetAsmJsFunctionInfo()->SetReturnType(Js::AsmJsRetType::Float);
            break;
        case WasmTypes::F64:
            retOp = Js::OpCodeAsmJs::Return_Db;
            m_func->body->GetAsmJsFunctionInfo()->SetReturnType(Js::AsmJsRetType::Double);
            break;
        case WasmTypes::I32:
            retOp = Js::OpCodeAsmJs::Return_Int;
            m_func->body->GetAsmJsFunctionInfo()->SetReturnType(Js::AsmJsRetType::Signed);
            break;
        default:
            Assume(UNREACHED);
        }

        m_writer.Conv(retOp, 0, retExprInfo.location);
        GetRegisterSpace(retExprInfo.type)->ReleaseLocation(&retExprInfo);
    }
    else
    {
        if (m_funcInfo->GetResultType() != WasmTypes::Void)
        {
            throw WasmCompilationException(L"Non-void result type must have return expression");
        }

        // TODO (michhol): consider moving off explicit 0 for return reg
        m_writer.AsmReg1(Js::OpCodeAsmJs::LdUndef, 0);
    }

    return EmitInfo();
}

Js::OpCodeAsmJs
WasmBytecodeGenerator::GetOpCodeForBinNode() const
{
    switch (m_reader->m_currentNode.op)
    {
#define WASM_KEYWORD_BIN_MATH(token, name, floatOp, doubleOp, intOp) \
    case wn##token: \
        switch(m_reader->m_currentNode.type) \
        { \
        case WasmTypes::F32: \
            return Js::OpCodeAsmJs::##floatOp; \
        case WasmTypes::F64: \
            return Js::OpCodeAsmJs::##doubleOp; \
        case WasmTypes::I32: \
            return Js::OpCodeAsmJs::##intOp; \
        default: \
            Assume(UNREACHED); \
        }

#include "WasmKeywords.h"

    default:
        Assume(UNREACHED);
    }
    return Js::OpCodeAsmJs::Nop;
}

Js::OpCodeAsmJs
WasmBytecodeGenerator::GetOpCodeForCompareNode() const
{
    switch (m_reader->m_currentNode.op)
    {
#define WASM_KEYWORD_COMPARE(token, name, floatOp, doubleOp, intOp) \
    case wn##token: \
        switch(m_reader->m_currentNode.type) \
        { \
        case WasmTypes::F32: \
            return Js::OpCodeAsmJs::##floatOp; \
        case WasmTypes::F64: \
            return Js::OpCodeAsmJs::##doubleOp; \
        case WasmTypes::I32: \
            return Js::OpCodeAsmJs::##intOp; \
        default: \
            Assume(UNREACHED); \
        }

#include "WasmKeywords.h"

    default:
        Assume(UNREACHED);
    }
    return Js::OpCodeAsmJs::Nop;
}

Js::AsmJsRetType
WasmBytecodeGenerator::GetAsmJsReturnType() const
{
    Js::AsmJsRetType asmType = Js::AsmJsRetType::Void;
    switch (m_funcInfo->GetResultType())
    {
    case WasmTypes::F32:
        asmType = Js::AsmJsRetType::Float;
        break;
    case WasmTypes::F64:
        asmType = Js::AsmJsRetType::Double;
        break;
    case WasmTypes::I32:
        asmType = Js::AsmJsRetType::Signed;
        break;
    case WasmTypes::Void:
        asmType = Js::AsmJsRetType::Void;
        break;
    default:
        Assert(UNREACHED);
    }
    return asmType;
}

Js::AsmJsVarType
WasmBytecodeGenerator::GetAsmJsVarType(WasmTypes::WasmType wasmType)
{
    Js::AsmJsVarType asmType = Js::AsmJsVarType::Int;
    switch (wasmType)
    {
    case WasmTypes::F32:
        asmType = Js::AsmJsVarType::Float;
        break;
    case WasmTypes::F64:
        asmType = Js::AsmJsVarType::Double;
        break;
    case WasmTypes::I32:
        asmType = Js::AsmJsVarType::Int;
        break;
    default:
        Assert(UNREACHED);
    }
    return asmType;
}

EmitInfo
WasmBytecodeGenerator::EmitBinExpr(WasmOp op)
{
    WasmTypes::WasmType type = m_reader->m_currentNode.type;

    Js::OpCodeAsmJs opcode = GetOpCodeForBinNode();

    EmitInfo lhs = EmitExpr(m_reader->ReadExpr());
    EmitInfo rhs = EmitExpr(m_reader->ReadExpr());

    if (type != lhs.type)
    {
        throw WasmCompilationException(L"Invalid type for LHS");
    }
    if (type != rhs.type)
    {
        throw WasmCompilationException(L"Invalid type for RHS");
    }


    if (opcode == Js::OpCodeAsmJs::Nop)
    {
        throw WasmCompilationException(L"Invalid type for operation");
    }

    WasmRegisterSpace * regSpace = GetRegisterSpace(type);

    Js::RegSlot resultReg = regSpace->AcquireRegisterAndReleaseLocations(&lhs, &rhs);

    m_writer.AsmReg3(opcode, resultReg, lhs.location, rhs.location);

    return EmitInfo(resultReg, type);
}

EmitInfo
WasmBytecodeGenerator::EmitCompareExpr(WasmOp op)
{

    WasmTypes::WasmType type = m_reader->m_currentNode.type;

    Js::OpCodeAsmJs opcode = GetOpCodeForCompareNode();

    EmitInfo lhs = EmitExpr(m_reader->ReadExpr());
    EmitInfo rhs = EmitExpr(m_reader->ReadExpr());

    if (type != lhs.type)
    {
        throw WasmCompilationException(L"Invalid type for LHS");
    }
    if (type != rhs.type)
    {
        throw WasmCompilationException(L"Invalid type for RHS");
    }

    if (opcode == Js::OpCodeAsmJs::Nop)
    {
        throw WasmCompilationException(L"Invalid type for operation");
    }

    WasmRegisterSpace * regSpace = GetRegisterSpace(type);

    Js::RegSlot resultReg = regSpace->AcquireRegisterAndReleaseLocations(&lhs, &rhs);

    m_writer.AsmReg3(opcode, resultReg, lhs.location, rhs.location);

    return EmitInfo(resultReg, type);
}

void
WasmBytecodeGenerator::GenerateInvoke()
{
    /*
    // TODO: this only works for single invoke

    // TODO: fix these bools
    m_writer.Begin(m_wasmScript->globalBody, &m_alloc, false, true, false);

    // TODO: loop through all arguments, not just 1, get arg count from parser
    EmitInfo argInfo = EmitExpr(invokeNode->invk.expr);

    const Js::ArgSlot argByteSize = sizeof(int32) + sizeof(Js::Var);
    m_writer.AsmStartCall(Js::OpCodeAsmJs::I_StartCall, argByteSize);
    m_writer.AsmReg2(Js::OpCodeAsmJs::ArgOut_Int, 1, argInfo.location);
    //m_writer.AsmSlot(Js::OpCodeAsmJs::LdSlot, 0, 1, index + mCompiler->GetFuncOffset());
    m_writer.AsmCall(Js::OpCodeAsmJs::I_Call, 0, 0, (Js::ArgSlot)(::ceil((double)(argByteSize / sizeof(Js::Var)))) + 1, Js::AsmJsRetType::Signed);

    m_writer.End();
    */
}

void WasmBytecodeGenerator::AddExport()
{
    if (m_reader->m_currentNode.var.num > m_module->functions->Count())
    {
        throw WasmCompilationException(L"Invalid index for export");
    }
    if (m_module->exports->AddNew(m_reader->m_currentNode.var.exportName, m_reader->m_currentNode.var.num) == -1)
    {
        throw WasmCompilationException(L"Unable to export function");
    }
}

WasmRegisterSpace *
WasmBytecodeGenerator::GetRegisterSpace(WasmTypes::WasmType type)
{
    WasmRegisterSpace * regSpace;
    switch (type)
    {
    case WasmTypes::F32:
        regSpace = &m_f32RegSlots;
        break;
    case WasmTypes::F64:
        regSpace = &m_f64RegSlots;
        break;
    case WasmTypes::I32:
        regSpace = &m_i32RegSlots;
        break;
    default:
        regSpace = nullptr;
        break;
    }
    return regSpace;
}

WasmCompilationException::WasmCompilationException(const wchar_t* _msg, ...)
{
    Assert(UNREACHED); // TODO (michhol)
    va_list arglist;
    va_start(arglist, _msg);
    //vswprintf_s(msg_, _msg, arglist);
}

} // namespace Wasm

#endif // ENABLE_WASM
