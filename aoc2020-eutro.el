;;; aoc2020-eutro.el --- Commands for Eutro's AoC 2020 in C -*- lexical-binding: t -*-

;;; Commentary:

;; AoC 2020 in C: https://github.com/eutropius225/aoc2020

;;; Code:

(require 'json)

(defgroup aoc2020-eutro nil
  "Group for aoc2020-eutro things."
  :prefix 'aoc-
  :group 'misc)

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

(defun aoc-get-session ()
  "Get the session key from root/session.key."
  (let ((file (concat aoc-root "session.key")))
    (or
     (when (file-readable-p file)
       (with-temp-buffer
         (insert-file-contents file)
         (buffer-substring (point-min) (point-max))))
     (error "Couldn't get session key"))))

(defun aoc-fetch (day)
  "Fetch the input data for DAY.

Requires a key file \"session.key\" in the root directory of this repo."
  (interactive "P")
  (let* ((day (or (and day (prefix-numeric-value day))
                  (read-number "Day: " (nth 3 (decode-time))))))
    (with-temp-buffer
      (with-temp-message "Fetching..."
        (shell-command (format
                        (concat
                         "cd %s && "
                         "curl -fs -H\"Cookie: session=%s\" "
                         "\"https://adventofcode.com/2020/day/%s/input\" "
                         "> input/%s.txt "
                         "&& printf Done || printf Errored!")
                        aoc-root
                        (aoc-get-session)
                        day
                        day)
                       (current-buffer)))
      (message (buffer-substring (point-min) (point-max))))))

(defconst aoc-keymap
  (let ((keys (make-sparse-keymap)))
    (define-key keys (kbd "C-c C-r") #'aoc-run)
    (define-key keys (kbd "C-c C-c") #'aoc-compile)
    (define-key keys (kbd "C-c C-f") #'aoc-fetch)
    (define-key keys (kbd "C-c C-l") #'aoc-leaderboard)
    keys))

(define-minor-mode aoc2020-eutro
  "Convenience commands for Eutro's AoC 2020 in C"
  nil
  " AoC-2020"
  aoc-keymap)

(defconst aoc-leaderboard-mode-map
  (let ((keys (make-sparse-keymap)))
    (define-key keys (kbd "RET") #'aoc-expand-star)
    (define-key keys (kbd "C-c C-l") #'aoc-leaderboard)
    (define-key keys (kbd "g") #'aoc-reload-leaderboard)
    (define-key keys (kbd "c") #'aoc-cycle-sort-mode)
    keys))

(define-derived-mode aoc-leaderboard-mode special-mode "AoC-Leaderboard"
  "A major mode for displaying an AoC leaderboard.")

(defface aoc-lit-number-face
  '((t . (:foreground "green")))
  "Face used for lit day numbers.")

(defface aoc-dim-number-face
  '((t . (:foreground "dark green")))
  "Face used for unlit day numbers.")

(defface aoc-leaderboard-title-face
  '((t . (:height 2.0 :foreground "lawn green")))
  "Face used for the title: \"%s's Leaderboard\".")

(defface aoc-day-header-face
  '((t . (:height 1.25 :foreground "lawn green")))
  "Face used for the subtitle \"Day %d\".")

(defface aoc-gray-star-face
  '((t . (:foreground "dim gray")))
  "Face used for no stars on a given day.")

(defface aoc-silver-star-face
  '((t . (:foreground "light gray")))
  "Face used for 1 star on a given day.")

(defface aoc-gold-star-face
  '((t . (:foreground "gold")))
  "Face used for both stars on a given day.")

(defconst aoc-star-faces
  '(aoc-gray-star-face
    aoc-silver-star-face
    aoc-gold-star-face))

(defvar-local aoc-leaderboard-data nil)
(defvar-local aoc-leaderboard-id nil)
(defvar-local aoc-sort-mode 'local)

(defun aoc-get-local (member)
  "Get the local score of MEMBER."
  (cdr (assoc 'local_score member)))

(defun aoc-sort-local (first second)
  "Sort FIRST and SECOND based on Local Score.

[Local Score], wards users on this leaderboard points
much like the global leaderboard.  If you add or remove users,
the points will be recalculated, and the order can change.
For N users, the first user to get each star gets N points,
the second gets N-1, and the last gets 1. This is the default."
  (> (aoc-get-local first)
     (aoc-get-local second)))

(defun aoc-get-global (member)
  "Get the global score of MEMBER."
  (cdr (assoc 'global_score member)))

(defun aoc-sort-global (first second)
  "Sort FIRST and SECOND based on Global Score.

[Global Score], which uses scores from the global leaderboard.
Ties are broken by the user's local score."
  (let ((first-global (aoc-get-global first))
        (second-global (aoc-get-global second)))
    (if (= first-global second-global)
        (aoc-sort-local first second)
      (> first-global second-global))))

(defun aoc-get-stars (member)
  "Get the stars of MEMBER."
  (cdr (assoc 'stars member)))

(defun aoc-get-last-star-ts (member)
  "Get the last star timestamp of MEMBER."
  (string-to-number (cdr (assoc 'last_star_ts member))))

(defun aoc-sort-stars (first second)
  "Sort FIRST and SECOND based on Stars.

[Stars], which uses the number of stars the user has.
Ties are broken by the time the most recent star was acquired.
This used to be the default."
  (let ((first-stars (aoc-get-stars first))
        (second-stars (aoc-get-stars second)))
    (if (= first-stars second-stars)
        (< (aoc-get-last-star-ts first)
           (aoc-get-last-star-ts second))
      (> first-stars second-stars))))

(defconst aoc-sort-funs
  '((local . aoc-sort-local)
    (global . aoc-sort-global)
    (stars . aoc-sort-stars)))

(defun aoc-get-name (member)
  "Get the display name of MEMBER."
  (cdr (assoc 'name member)))

(defun aoc-insert-local (member)
  "Insert the local score of MEMBER at point."
  (insert (format "% 4d" (aoc-get-local member))))

(defun aoc-insert-global (member)
  "Insert the globals score of MEMBER at point."
  (insert (format "% 4d" (aoc-get-global member))))

(defun aoc-insert-stars (member)
  "Insert the star count of MEMBER at point."
  (insert (propertize (format "%3d*" (aoc-get-stars member))
                      'face 'aoc-gold-star-face)))

(defconst aoc-sort-inserters
  '((local . aoc-insert-local)
    (global . aoc-insert-global)
    (stars . aoc-insert-stars)))

(defun aoc-show-day-p (day)
  "Return non-nil if DAY has passed."
  (let ((time (decode-time (current-time) -18000)))
    (or (> (nth 5 time) 2020)
        (>= (nth 3 time) day))))

(defun aoc-insert-star (member day)
  "Insert a the star of MEMBER for DAY at point."
  (let ((star-data (cdr (assoc (intern (number-to-string day))
                               (assoc 'completion_day_level member)))))
    (insert
     (if (aoc-show-day-p day)
         (propertize "*" 'face (nth (length star-data) aoc-star-faces))
       " "))
    (let ((overlay (make-overlay (1- (point)) (point) (current-buffer) nil nil)))
      (overlay-put overlay 'aoc-star-data (cons day star-data)))))

(defun aoc-get-star-data-overlay ()
  "Get the star data overlay at point."
  (seq-find (lambda (overlay)
              (overlay-get overlay 'aoc-star-data))
            (overlays-at (point))))

(defcustom aoc-time-string "%c"
  "Time string formatted when expanding a star."
  :type 'stringp
  :group aoc2020-eutro)

(defun aoc-expand-star ()
  "Expand the star at point.

If the star was just expanded, collapse it instead."
  (interactive)
  (let ((star-overlay (aoc-get-star-data-overlay)))
    (when (and star-overlay (= ?* (char-after)))
      (save-excursion
        (end-of-line)
        (read-only-mode -1)
        (let ((start (point))
              (star-data (overlay-get star-overlay 'aoc-star-data))
              (prev-overlay (aoc-get-star-data-overlay)))
          (when prev-overlay
            (delete-region start (overlay-end prev-overlay)))
          (unless (and prev-overlay
                       (eq star-data (overlay-get prev-overlay 'aoc-star-data)))
            (newline)
            (insert
             "  "
             (propertize (format "Day %d" (car star-data))
                         'face 'aoc-day-header-face))
            (dotimes (i 2)
              (let ((sym (intern (number-to-string (1+ i)))))
                (when (assoc sym star-data)
                  (newline)
                  (insert
                   (propertize (format "   %s* " sym) 'face 'aoc-gold-star-face)
                   (let ((timestamp (string-to-number (cdadr (assoc sym star-data)))))
                     (format-time-string aoc-time-string timestamp))))))
            (newline)
            (overlay-put (make-overlay start (point) (current-buffer) nil nil)
                         'aoc-star-data
                         (overlay-get star-overlay 'aoc-star-data))))
        (read-only-mode t))
      (point))))

(defun aoc-insert-member (member)
  "Insert information about MEMBER at point."
  (funcall (cdr (assoc aoc-sort-mode aoc-sort-inserters))
           member)
  (insert " ")
  (dotimes (i 25)
    (aoc-insert-star member (1+ i)))
  (insert " ")
  (insert (or (aoc-get-name member) "-")))

(defun aoc-insert-day-number (day)
  "Insert the DAY number at point and the next line."
  (let ((face (if (aoc-show-day-p day)
                  'aoc-lit-number-face
                'aoc-dim-number-face)))
    (insert (propertize
             (if (>= day 10)
                            (number-to-string (floor (/ day 10)))
               " ")
             'face face))
    (end-of-line 2)
    (insert (propertize
             (number-to-string (% day 10))
             'face face))))

(defun aoc-populate-leaderboard-buffer ()
  "Populate the leaderboard buffer with names, stars and stuff."
  (read-only-mode -1)
  (erase-buffer)
  (let* ((members (cdr (assoc 'members aoc-leaderboard-data)))
         (owner-id-sym (intern (number-to-string aoc-leaderboard-id)))
         (owner-name (aoc-get-name (assoc owner-id-sym members))))
    (insert (propertize (format "%s's Leaderboard" owner-name)
                        'face 'aoc-leaderboard-title-face))
    (newline)
    (insert "     \n"
            "     ")
    (dotimes (i 25)
      (end-of-line 0)
      (aoc-insert-day-number (1+ i)))
    (goto-char (point-max))
    (let ((members-sorted (sort members (cdr (assoc aoc-sort-mode aoc-sort-funs)))))
      (while members-sorted
        (newline)
        (aoc-insert-member (pop members-sorted)))))
  (newline)
  (read-only-mode t))

(defun aoc-cycle-sort-mode ()
  "Cycle the sort mode on the leaderboard."
  (interactive)
  (unless (eq major-mode 'aoc-leaderboard-mode)
    (error "Not in leaderboard buffer"))
  (setq aoc-sort-mode
        (cdr (assoc aoc-sort-mode
                    '((local . global)
                      (global . stars)
                      (stars . local)))))
  (aoc-reload-leaderboard))

(defun aoc-fetch-leaderboard-data ()
  "Fetch the AoC leaderboard data of `aoc-leaderboard-id'.

The data is then stored in `aoc-leaderboard-data'."
  (unless aoc-leaderboard-id
    (error "No leaderboard ID"))
  (setq
   aoc-leaderboard-data
   (let ((id aoc-leaderboard-id))
     (with-temp-buffer
       (shell-command
        (format
         (concat
          "curl -fs -H\"Cookie: session=%s\" "
          "https://adventofcode.com/2020/leaderboard/private/view/%s.json")
         (aoc-get-session)
         id)
        (current-buffer))
       (json-read)))))

(defun aoc-reload-leaderboard ()
  "Reload the leaderboard."
  (interactive)
  (unless (eq major-mode 'aoc-leaderboard-mode)
    (error "Not in leaderboard buffer"))
  (aoc-fetch-leaderboard-data)
  (aoc-populate-leaderboard-buffer))

(defun aoc-leaderboard (id &optional buffer)
  "Display the private leaderboard ID in BUFFER.

If BUFFER is nil, display in a new buffer *AoC Leaderboard*."
  (interactive "nID: ")
  (let* ((buffer (or buffer "*AoC Leaderboard*")))
    (with-current-buffer (get-buffer-create buffer)
      (aoc-leaderboard-mode)
      (setq aoc-leaderboard-id id)
      (aoc-reload-leaderboard)
      (display-buffer (current-buffer)))))

(provide 'aoc2020-eutro)
;;; aoc2020-eutro.el ends here
