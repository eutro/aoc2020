((nil
  .
  ((indent-tabs-mode . nil)
   (eval
    .
    (defun aoc-compile-and-run ()
      (interactive)
      (compile (concat "cd "
		       (locate-dominating-file (eval-when-compile
						 (or load-file-name
						     (buffer-file-name)))
					       "Makefile")
		       " && make -k && ./aoc")))))))
