;;; ecasound.el --- Ecasound major mode for inferior ecasound processes

;; Copyright (C) 2001  Mario Lang 

;; Author: Mario Lang <mlang@delysid.org>
;; Keywords: audio, ecasound, comint, process

;; This file is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; This file is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;; Commentary:

;; Implements a derived-major-mode, from comint mode
;; for an inferior ecasound process.
;;
;; Uses pcomplete to offer context sensitive completion/help.
;;
;; Todo:
;;
;; Much...

;;; Code:

(require 'comint)
(require 'pcomplete)

(defgroup ecasound nil
  "Variables related to ecasound inferior processes."
  :prefix "ecasound-")

(defcustom ecasound-arguments '("-c")
  "*Default command line arguments when starting a ecasound process."
  :group 'ecasound
  :type '(repeat string))

(defvar ecasound-mode-map
  (let ((map (nconc (make-sparse-keymap) comint-mode-map)))
    (define-key map "\t" 'comint-dynamic-complete)
    (define-key map "\M-?"
      'comint-dynamic-list-filename-completions)
    (define-key map [menu-bar completion]
      (copy-keymap (lookup-key comint-mode-map [menu-bar completion])))
    map))

(defcustom ecasound-program "/usr/bin/ecasound"
  "Ecasound's executable."
  :group 'ecasound
  :type 'file)

(defcustom ecasound-prompt-regexp "^ecasound[^>]*> "
  "Regexp to use to match the prompt."
  :group 'ecasound
  :type 'regexp)

(defconst ecasound-commands
  '(;; GENERAL
    "quit" "q"
    "start" "t"
    "stop" "s"
    "run"
    "debug"
    "help" "h"
    ;; GLOBAL
    "status" "st"
    "engine-status"
    ;; CHAINSETUPS
    "cs-add"
    "cs-remove"
    "cs-list"
    "cs-select"
    "cs-selected"
    "cs-index-select" "cs-iselect"
    "cs-load"
    "cs-save"
    "cs-save-as"
    "cs-edit"
    "cs-is-valid"
    "cs-connect"
    "cs-disconnect"
    "cs-connected"
    "cs-rewind" "rewind" "rw"
    "cs-forward" "forward" "fw"
    "cs-set-position" "cs-setpos" "setpos" "set-position"
    "cs-get-position" "cs-getpos" "getpos" "get-position"
    "cs-get-length" "get-length"
    "cs-set-length"
    "cs-toggle-loop"
    "cs-set-param"
    "cs-set-audio-format"
    "cs-status" "cs"
    ;; CHAINS
    "c-add"
    "c-remove"
    "c-list"
    "c-select"
    "c-index-select" "c-iselect"
    "c-select-all"
    "c-select-add"
    "c-deselect"
    "c-selected"
    "c-clear"
    "c-rename"
    "c-muting"
    "c-bypass"
    "c-forward" "c-fw"
    "c-rewind" "c-rw"
    "c-set-position" "c-setpos"
    "c-status"
    ;; AUDIO INPUT/OUTPUT OBJECTS
    "ai-add"
    "ao-add"
    "ai-select"
    "ao-select"
    "ai-index-select" "ai-iselect"
    "ao-index-select" "ao-iselect"
    "ai-selected"
    "ao-selected"
    "ai-attach"
    "ao-attach"
    "ai-remove"
    "ao-remove"
    "ai-forward" "ai-fw"
    "ao-forward" "ao-fw"
    "ai-rewind" "ai-rw"
    "ao-rewind" "ao-rw"
    "ai-set-position" "ai-setpos"
    "ao-set-position" "ao-setpos"
    "ai-get-position" "ai-getpos"
    "ao-get-position" "ao-getpos"
    "ai-get-length"
    "ao-get-length"
    "ai-get-format"
    "ao-get-format"
    "ai-wave-edit"
    "ao-wave-edit"
    "ai-list"
    "ao-list"
    "aio-register"
    "aio-status"
    ;; CHAIN OPERATORS
    "cop-add"
    "cop.list"
    "cop-remove"
    "cop-select"
    "cop-index-select" "cop-iselect"
    "cop-selected"
    "cop-set"
    "cop-status"
    "copp-list"
    "copp-select"
    "copp-index-select" "copp-iselect"
    "copp-selected"
    "copp-set"
    "copp-get"
    "cop-register"
    "preset-register"
    "ladspa-register"
    ;; CONTROLLERS
    "ctrl-add"
    "ctrl-remove"
    "ctrl-list"
    "ctrl-select"
    "ctrl-index-select" "ctrl-iselect"
    "ctrl-selected"
    "ctrl-status"
    "ctrl-register"
    ;; OBJECT MAPS
    ; Unimplemen5ted currently...
    )
  "Available Ecasound IAM commands.")
  
(define-derived-mode ecasound-mode comint-mode "Ecasound"
  (set (make-local-variable 'comint-prompt-regexp)
       (set (make-local-variable 'paragraph-start)
            ecasound-prompt-regexp))
  (ecasound-setup-pcomplete))

(defun ecasound-setup-pcomplete ()
  (set (make-local-variable 'pcomplete-command-completion-function)
       'ecasound-command-completion-function)
  (set (make-local-variable 'pcomplete-command-name-function)
       (lambda ()
         (pcomplete-arg 'first)))
  (pcomplete-comint-setup 'comint-dynamic-complete-functions))

(defun ecasound-command-completion-function ()
  (pcomplete-here
   ecasound-commands))

;;;###autoload
(defun ecasound (&optional buffer)
  "Run an inferior ecasound, with I/O through BUFFER (which defaults to `*ecasound*').
Interactively, a prefix arg means to prompt for BUFFER.
If BUFFER exists but shell process is not running, make new shell.
If BUFFER exists and shell process is running, just switch to BUFFER.
The buffer is put in ecasound mode, giving commands for sending input.
See `ecasound-mode'.

\(Type \\[describe-mode] in the ecasound buffer for a list of commands.)"
  (interactive
   (list
    (and current-prefix-arg
         (read-buffer "Ecasound buffer: " "*ecasound*"))))
  (when (null buffer)
    (setq buffer "*ecasound*"))
  (if (not (comint-check-proc buffer))
      (let (ecasound-buffer)
        (save-excursion
          (set-buffer (apply 'make-comint-in-buffer
                             "ecasound" buffer
                             ecasound-program
                             nil
                             ecasound-arguments))
          (setq ecasound-buffer (current-buffer))
          (ecasound-mode))
        (pop-to-buffer ecasound-buffer))
    (pop-to-buffer buffer)))

(provide 'ecasound)

;;; ecasound.el ends here