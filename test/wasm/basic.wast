;;-------------------------------------------------------------------------------------------------------
;; Copyright (C) Microsoft. All rights reserved.
;; Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
;;-------------------------------------------------------------------------------------------------------

(module
  (func (param i32) (result i32)
    (if (i32.ges (i32.const 26) (i32.const 25)) (setlocal 0 (i32.add (getlocal 0) (i32.const 4))))
    (block
    (setlocal 0 (i32.add (getlocal 0) (i32.const 4)))
    (setlocal 0 (i32.add (getlocal 0) (i32.const 4)))
    (setlocal 0 (i32.add (getlocal 0) (i32.const 4))))
    (return (i32.add (getlocal 0) (i32.const 42)))
  )

  (export "a" 0)
)
