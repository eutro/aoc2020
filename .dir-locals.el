((nil
  .
  ((indent-tabs-mode . nil)
   (eval . (unless (featurep 'aoc2020-eutro)
             (let ((load-path (cons (locate-dominating-file
                                     (or load-file-name
                                         (buffer-file-name))
                                     "aoc2020-eutro.el")
                                    load-path)))
               (require 'aoc2020-eutro))))
   (eval . (aoc2020-eutro))))
 (makefile-gmake-mode . ((indent-tabs-mode . t))))
