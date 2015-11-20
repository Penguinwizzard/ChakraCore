;;-------------------------------------------------------------------------------------------------------
;; Copyright (C) Microsoft. All rights reserved.
;; Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
;;-------------------------------------------------------------------------------------------------------

(module
  (func (param i32) (result i32)
    (if (ges.i32 (const.i32 26) (const.i32 25)) (setlocal 0 (add.i32 (getlocal 0) (const.i32 4))))
    (block
    (setlocal 0 (add.i32 (getlocal 0) (const.i32 4)))
    (setlocal 0 (add.i32 (getlocal 0) (const.i32 4)))
    (setlocal 0 (add.i32 (getlocal 0) (const.i32 4))))
    (return (add.i32 (getlocal 0) (const.i32 42)))
  )

  (export "a" 0)
)
