//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// This is a variant of the prototype regex for finding CSS classes
// that was causing problems on classes
// with -'s in their names 
WScript.Echo("b-b".match(/([\w\-\*]+)\b/));

// Here is a simpler repro of the above:
WScript.Echo("one two".match(/.*\b/));
