;;; aoc2020-eutro.el -- Convenience commands for Eutro's AoC 2020 in C

;;; Commentary:

;; AoC 2020 in C: https://github.com/eutropius225/aoc2020

;;; Code:

(defconst aoc-root
  (locate-dominating-file
   (or load-file-name
       (buffer-file-name))
   "Makefile"))

(defun aoc-compile-command (&rest args)
  "Get the command to compile with.

ARGS are extra arguments to pass to make."
  (format "cd %s && make%s"
          aoc-root
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
                       (number-to-string
                        (if (called-interactively-p 'interactive)
                            (if (numberp day)
                                day
                              (read-number "Day: " (nth 3 (decode-time))))
                          day)))
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

(defun aoc-fetch (day)
  "Fetch the input data for DAY.

Requires a key file \"session.key\" in the root directory of this repo."
  (interactive "P")
  (let* ((day (or (and day (prefix-numeric-value day))
                  (read-number "Day: " (nth 3 (decode-time)))))
         (file (concat aoc-root "session.key"))
         (session
          (when (file-readable-p file)
            (with-temp-buffer
              (insert-file-contents file)
              (buffer-substring (point-min) (point-max))))))
    (if (not session)
        (message "%s" "Couldn't get session.key")
      (with-temp-buffer
        (with-temp-message "Fetching..."
          (shell-command (format
                          (concat
                           "cd %s && "
                           "curl -fs -H\"Cookie: session=%s\" "
                           "\"https://adventofcode.com/2020/day/%s/input\" "
                           "> input/%s.txt "
                           "&& printf \"Done\" || printf \"Errored!\"")
                          aoc-root
                          session
                          day
                          day)
                         (current-buffer)))
        (message (buffer-substring (point-min) (point-max)))))))

(defconst aoc-keymap
  (let ((keys (make-sparse-keymap)))
    (define-key keys (kbd "C-c C-r") #'aoc-run)
    (define-key keys (kbd "C-c C-c") #'aoc-compile)
    (define-key keys (kbd "C-c C-f") #'aoc-fetch)
    keys))

(define-minor-mode aoc2020-eutro
  "Convenience commands for Eutro's AoC 2020 in C"
  nil
  " AoC-2020"
  aoc-keymap)

(provide 'aoc2020-eutro)
;;; aoc2020-eutro.el ends here
