((nil
  .
  ((indent-tabs-mode . nil)
   (eval
    .
    (defun aoc-compile-command (&rest args)
      "Get the command to compile with.

ARGS are extra arguments to pass to make."
      (format "cd %s && make%s"
              (locate-dominating-file
               (eval-when-compile
                 (or load-file-name
                     (buffer-file-name)))
               "Makefile")
              (apply #'concat
                     (mapcar (lambda (arg)
                               (concat " " arg))
                             args)))))
   (eval
    .
    (defun aoc-run (&optional day)
      "Compile and run the program for the given DAY.

When called interactively, with a prefix argument,
if it is raw (explicitly specified), use that as DAY,
otherwise read DAY from the minibuffer.
Without a prefix argument, run all days."
      (interactive "P")
      (compile
       (format "%s && ./aoc%s"
               (aoc-compile-command "-s")
               (if day
                   (concat " "
                           (if (called-interactively-p)
                               (if (numberp day)
                                   (number-to-string day)
                                 (read-string "Day: "))
                             (number-to-string day)))
                 "")))))
   (eval
    .
    (defun aoc-compile (&optional args)
      "Compile the program with the given string arguments ARGS.

If ARGS is not supplied, compile with just \"-k\".

When called interactively with a prefix argument, read the arguments
from the minibuffer."
      (interactive "P")
      (compile (aoc-compile-command
                (if args
                    (if (called-interactively-p)
                        (read-string "Arguments: ")
                      args)
                  "-k")))))
   (eval
    .
    (progn
      (local-set-key (kbd "C-c C-r") #'aoc-run)
      (local-set-key (kbd "C-c C-c") #'aoc-compile)))))
 (makefile-gmake-mode . ((indent-tabs-mode . t))))
