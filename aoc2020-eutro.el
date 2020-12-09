;;; aoc2020-eutro.el -- Convenience commands for Eutro's AoC 2020 in C

;;; Commentary:

;; AoC 2020 in C: https://github.com/eutropius225/aoc2020

;;; Code:

(defun aoc-compile-command (&rest args)
  "Get the command to compile with.

ARGS are extra arguments to pass to make."
  (format "cd %s && make%s"
          (eval-when-compile
            (locate-dominating-file
             load-file-name
             "Makefile"))
          (apply #'concat
                 (mapcar (lambda (arg)
                           (concat " " arg))
                         args))))

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
                       (if (called-interactively-p 'interactive)
                           (if (numberp day)
                               (number-to-string day)
                             (read-string "Day: "))
                         (number-to-string day)))
             ""))))

(defun aoc-compile (&optional args)
  "Compile the program with the given string arguments ARGS.

If ARGS is not supplied, compile with just \"-k\".

When called interactively with a prefix argument, read the arguments
from the minibuffer."
  (interactive "P")
  (compile (aoc-compile-command
            (if args
                (if (called-interactively-p 'interactive)
                    (read-string "Arguments: ")
                  args)
              "-k"))))

(defconst aoc-keymap
  (let ((keys (make-sparse-keymap)))
    (define-key keys (kbd "C-c C-r") #'aoc-run)
    (define-key keys (kbd "C-c C-c") #'aoc-compile)
    keys))

(define-minor-mode aoc2020-eutro
  "Convenience commands for Eutro's AoC 2020 in C"
  nil
  " AoC-2020"
  aoc-keymap)

(provide 'aoc2020-eutro)
;;; aoc2020-eutro.el ends here
