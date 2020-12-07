((nil
  .
  ((indent-tabs-mode . nil)
   (eval
    .
    (defun aoc-compile-command (&rest flags)
      (concat "cd "
              (locate-dominating-file (eval-when-compile
                                        (or load-file-name
                                            (buffer-file-name)))
                                      "Makefile")
              " && make"
              (apply #'concat
                     (mapcar (lambda (flag)
                               (concat " " flag))
                             flags)))))
   (eval
    .
    (defun aoc-all ()
      (interactive)
      (compile (concat (aoc-compile-command "-s")
		       " && ./aoc"))))
   (eval
    .
    (defun aoc-day (day)
      (interactive "nDay: ")
      (compile (concat (aoc-compile-command "-s")
		       " && ./aoc "
		       (number-to-string day)))))
   (eval
    .
    (defun aoc-compile ()
      (interactive)
      (compile (aoc-compile-command "-k"))))))
 (makefile-gmake-mode . ((indent-tabs-mode . t))))
