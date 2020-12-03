((nil
  .
  ((indent-tabs-mode . nil)
   (eval
    .
    (defun aoc-compile-command ()
      (concat "cd "
              (locate-dominating-file (eval-when-compile
                                        (or load-file-name
                                            (buffer-file-name)))
                                      "Makefile")
              " && make -k"))
    (defun aoc-all ()
      (interactive)
      (compile (concat (aoc-compile-command)
		       " && ./aoc")))
    (defun aoc-day (day)
      (interactive "nDay: ")
      (compile (concat (aoc-compile-command)
		       " && ./aoc "
		       (number-to-string day)))))))
 (makefile-gmake-mode . ((indent-tabs-mode . t))))
