//Configuration: lessmath.xml
//Testcase Number: 18825
//Bailout Testing: ON
//Switches: -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist
//Baseline Switches:
//Arch: X86
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 150611-2131
//FullBuild: 10142.0.150611
//MachineName: BPT12319
//InstructionSet:
//reduced switches: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist
//noRepro switches1: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:InterpreterProfile
//noRepro switches2: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:ReJIT
//noRepro switches3: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:JITLoopBody
//noRepro switches4: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:DynamicProfile
//noRepro switches5: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:FieldCopyProp
//noRepro switches6: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:FloatTypeSpec
//noRepro switches7: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:AggressiveIntTypeSpec
//noRepro switches8: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:TypeSpec
//noRepro switches9: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:InlineBuiltIn
//noRepro switches10: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:Inline
//noRepro switches11: -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist -off:BailOnNoProfile
var obj0 = {};
var arrObj0 = {};
var func0 = function () {
  function func5() {
  }
  var uniqobj0 = func5(--arrObj0.prop1);
  return arrObj0;
};
var func2 = function () {
  do {
    var __loopSecondaryVar1000_0 = 3;
    for (; __loopSecondaryVar1000_0; __loopSecondaryVar1000_0--) {
      if (393855545) {
        ary.push(5);
      }
      v9 = arrObj0.prop1;
      ary[v9] = arrObj0;
    }
  } while (func0());
};
obj0.method1 = func2;
var ary = Array();
arrObj0.prop1 = 4294967297;
var uniqobj15 = [obj0];
try {
  uniqobj15[0].method1();
  print("FAILED");
} catch(e) {
  if(e instanceof RangeError) {
    print("PASSED");
  } else {
    print("Invalid error type");
    print("FAILED");
  }
}

// === Output ===
// command: D:\BinariesCache\fbl_ie_stage_dev3\checkins\1551126\x86chk\JsHost.exe -maxsimplejitruncount:2 -maxinterpretcount:1 -forcejitloopbody -off:arraycheckhoist step1270.js
// exitcode: C0000420
// stdout:
//
// stderr:
// ASSERTION 2396: (d:\enlist\dev3\inetcore\jscript\lib\runtime\language\javascriptexceptionoperators.cpp, line 832) !scriptContext || !scriptContext->GetThreadContext()->IsDisableImplicitException()
//  Failure: (!scriptContext || !scriptContext->GetThreadContext()->IsDisableImplicitException())
// FATAL ERROR: jshost.exe failed due to exception code c0000420
//
//
//
