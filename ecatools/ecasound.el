;;; ecasound.el --- Interactive and programmatic interface to Ecasound

;; Copyright (C) 2001, 2002  Mario Lang 

;; Author: Mario Lang <mlang@delysid.org>
;; Keywords: audio, ecasound, eci, comint, process, pcomplete
;; Version: 0.8.1

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

;; This file implements several aspects of ecasound use:
;;
;; * A derived-major-mode, from comint mode for an inferior ecasound
;; process (ecasound-aim-mode).  Complete with context sensitive
;; completion and interactive features to control the current process
;; using ECI.
;;
;; * Ecasound Control Interface (ECI) library for programmatic control
;; of a Ecasound process.  This allows you to write Ecasound batch
;; jobs in Emacs-Lisp with Lisp functions and return values.  Have a
;; look at eci-example and ecasound-normalize.
;;
;; * ecasound-ewf-mode, a mode for editing .ewf files.
;;
;;
;; Usage:
;;
;; You need a very recent version of ecasound for this file to work correclty.
;; Something >=2.2.0rc1, which currently means you need to compile it
;; from CVS.
;;
;; Put ecasound.el in your load-path and require it in your .emacs.
;; Set `ecasound-program' to the path to your ecasound executable.
;;
;;  (setq load-path (cons "/home/user/elisp")
;;  (require 'ecasound)
;;  (setq ecasound-program "/home/user/bin/ecasound"
;;        eci-program "/home/user/bin/ecasound")
;;
;; To set ecasound startup options use
;;
;;  M-x ecasound-customize-startup RET
;;
;; Then use M-x ecasound RET to invoke an inferior ecasound process.
;;
;; For programmatic use of the ECI API, have a look at `eci-init',
;; `eci-command' and in general the eci-* namespace.
;;
;; Compatibility:
;;
;; This file only works with GNU Emacs 21.  I've invested some minimal efforts
;; to get it working with XEmacs, but have so far failed to succeed.
;; Motivation isn't very high to get it working with XEmacs since I personally
;; never use it.  So if you would like to use ecasound.el under XEmacs, you
;; will have ttodo
;;  M-x toggle-debug-on-error RET
;; and see what you can figure out.  I'm happy to receive useful suggestions.
;;
;; Todo:
;;
;; * Change ecasound-cop-edit to use eci-cop-set.
;; * Add more conditions to the menu.
;; * Add timer-watchdogs.  On ecasound-quit, on buffer-kill, and so on.
;; * Use map-xxx-list data in the ecasound-copp widget.  This means we
;;   need to merge cop-status and map-cop-list data somehow or have
;;   the cop-editor fetch hints from map-cop/ladpsa/preset-list.
;; * Make `ecasound-signalview' faster, and allow to invoke it on already
;;   opened sessions.
;; * Fix the case where ecasound sends output *after* the prompt.
;;   This is tricky!  Fixed for internal parsing, probably will leave
;;   like that for interactive use, not worth the trouble...
;; * Copy documentation for ECI commands into eci-* docstrings and menu
;;   :help keywords.
;; * Expand the menu.
;; * Bind most important interactive functions in ecasound-iam-mode-map
;;   (which layout to use?)

;;; History:
;; 
;; Version: 0.8.1
;;
;; * Make ai|ao|cs-forward|rewind use ai|ao|cs-selected in the prompt
;;   string of the interactive spec.
;; * New keymaps ecasound-audioin|audioout-map.
;;   Now you can be very quick:
;;  M-x ecasound RET M-i a <select file> RET M-o d start RET
;; * New menu ecasound-iam-ai|ao-menu.
;; * defeci for ai|ao-add|forward|iselect|list|rewind|select|selected
;; * Deleted `ecasound-buffer-name' and `eci-buffer-name' and replaced
;;   calls to `make-comint-in-buffer' with `make-comint'.
;; * Extended defeci's :chache and :cache-doc to defvar the variable.
;; * Cleaned up some old alias definitions.
;;
;; Version: 0.8.0
;;
;; * New custom type ecasound-args, which is now used for `ecasound-arguments'
;;   and `eci-arguments'.
;; * If :cache is specified, also try to find a cached version in daemon-buffer
;;   if available.
;; * Added :alias keyword to defeci.  Delete defecialias.
;; * Added ":pcomplete doc" to several defeci calls.
;; * ecasound-cop|ctrl-add deleted and merged with the interactive spec of
;;   eci-cop|ctrl-add.  Now if prefix arg (C-u) is given, prompt for plain
;;   string, otherwise prompt with completion. Also changed binding
;;   in ChainOp menu.
;; * `ecasound-messages': variable deleted.
;; * `ecasound-arguments': Now handles -d:nnn properly.
;; * Many other minor tweaks and fixes.
;;
;; Version: 0.7.9
;;
;; * Cleanup and extend `defeci', now handles keyword :cache and :pcomplete.
;;   Lots of `defeci'-caller updates, and additions.
;; * Extended `ecasound-arguments' customize defition to handle --daemon,
;; --daemon-port:nnn, -n:name and -b:size.  New interactive function
;; `ecasound-customize-startup', also bound in "Ecasound menu."
;; * Added status-information fetching via timer-function.  Puts
;; info in mode-line as well as header-line. (warning, this feature is still
;; a bit unstable.)
;; * New macro `eci-hide-output' used to redirect action to `ecasound-daemon'
;; if possible.  Several completion-fascilities updated to use it.
;; * Various other fixes.
;;
;; Version: 0.7.8
;;
;; * Fix bug in "cop-add -el:" completion.
;; * Made `ecasound-format-arg' a bit prettier.
;; * Add --daemon support.  If --daemon is set in `ecasound-arguments',
;; ecasound-iam-mode will take advantage of that and initialize a
;; `ecasound-daemon' channel, as well as a periodic timer to update the
;; mode-line.  M-: (display-buffer ecasound-daemon) RET to view its contents.
;;
;; Version: 0.7.7
;;
;; * Fixed hangup if a Stringlist ('S') returned a empty list.
;; * Added keybindings.  See C-h m for details.  Still alot missing.
;; * Added cs-forward and cs-rewind.  Can be used interactively, or
;; prompt for value.  With no prefix arg, prompts for value, with
;; prefix arg, uses that.  Example: C-u M-c M-s f forwards the chainsetup
;; by 4 seconds, M-9 M-c M-s f forwards nine seconds ...
;; * Fixed field-no-longer-editable bug when +/- is used in
;; ecasound-cop-editor (thanks Per).  This also makes the slider useful again.
;; * Got rid of ecasound-prompt assumptions in `eci-parse' and `eci-command'.
;; * Make the eci-command family work with --daemon tcp/ip connections.
;;   (no code for initialising daemon stuff yet, but eci-* commands
;;    work fine with tcp/ip conns (tested manually).
;; * `eci-parse' deleted and merged with `eci-output-filter'.
;;
;; Version: 0.7.6
;;
;; * Various minor bugfixes and enhancements.
;; * Implemented ecasignalview as `ecasound-signalview' directly in Lisp.
;; This is another demonstration that ECI was really a Good Thing(tm)!
;; * Changed defeci to make it look more like a defun.
;; * Removed eci-process-*-register handling completely. Rationale is
;; that the map-*-list stuff is actually much more uniform and offers more
;; info.
;; * Rewrote `pcomplete/ecasound-iam-mode/cop-add' to use map-*-list.
;; * Rewrote `ecasound-ctrl-add' using map-ctrl-list instead of ctrl-register
;; and `ecasound-read-copp'.
;; * Rewrote `ecasound-cop-add' using map-cop|ladspa|preset-list.
;; * New function `eci-process-map-list' which processes the new map-xxx-list
;; output into a wellformed Lisp list.
;; * `ecasound-iam-commands' is now filled using int-cmd-list.
;; * cop-map-list handling.  Used in `ecasound-cop-add' now.  New function
;; `ecasound-read-copp' uses the now available default value.
;;
;; Version: 0.7.5
;;
;; * Added ctrl-register parsing support and `ecasound-ctrl-add'.
;; * Added preset-register support (so far only for cop-add completion)
;; * Fixed cop-status parsing bug which caused `ecasound-cop-edit' to not
;; work in some cases.
;; * New macro defeci which handles defining ECI commands in lisp.
;; * Several other minor tweaks and fixes.
;;
;; Version: 0.7.4
;;
;; * Fixed `eci-command' once again, it blocked for nearly every call... :(
;; * Fixed ecasound-cop-add in the ladspa case.
;;
;; Version: 0.7.3
;;
;; * Fixed missing require.
;;
;; Version: 0.7.2
;;
;; * Integrated ladspa-register into ecasound-cop-add
;; Now we've a very huge list to select from using completion.
;; * Some little cleanups.
;; * Fixed ecasound-cop-add to actually add the ':' between name and args.
;; * Removed the slider widget for now from the :format property of
;; ecasound-copp.
;; * Added `ecasound-messages' for a nice customisable interface to
;; loglevels, strangely, cvs version doesnt seem to recognize
;; -d:%d
;;
;; Version: 0.7.1
;;
;; * Created a slider widget.  It's not flawless, but it works!
;;

;;; Code:

(require 'cl)
(require 'comint)
(require 'easymenu)
(require 'pcomplete)
(require 'widget)
(require 'wid-edit)

(defgroup ecasound nil
  "Ecasound is a software package designed for multitrack audio processing.
It can be used for simple tasks like audio playback, recording and format
conversions, as well as for multitrack effect processing, mixing, recording
and signal recycling.  Ecasound supports a wide range of audio inputs, outputs
and effect algorithms.  Effects and audio objects can be combined in various
ways, and their parameters can be controlled by operator objects like
oscillators and MIDI-CCs.

Variables in this group affect inferior ecasound processes started from
within Emacs using the command `ecasound'.

See the subgroup `eci' for settings which affect the programmatic interface
to ECI."
  :prefix "ecasound-"
  :group 'processes)

(define-widget 'ecasound-cli-arg 'string
  "A Custom Widget for a command-line argument."
  :format "%t: %v%d"
  :string-match 'ecasound-cli-arg-string-match
  :match 'ecasound-cli-arg-match
  :value-to-internal
  (lambda (widget value)
    (when (widget-apply widget :string-match value)
      (match-string 1 value)))
  :value-to-external
  (lambda (widget value)
    (format (widget-apply widget :arg-format) value)))

(defun ecasound-cli-arg-match (widget value)
  (when (stringp value)
    (widget-apply widget :string-match value)))

(defun ecasound-cli-arg-string-match (widget value)
  (string-match
   (format (concat "^" (regexp-quote (widget-get widget :arg-format)))
	   (concat "\\(" (widget-get widget :pattern) "\\)"))
   value))

(define-widget 'ecasound-daemon-port 'ecasound-cli-arg
  "A Custom Widget for the --daemon-port:port argument."
  :pattern ".*"
  :arg-format "--daemon-port:%s")

(define-widget 'ecasound-chainsetup-name 'ecasound-cli-arg
  "A Custom Widget for the -n:chainsetup argument."
  :arg-format "-n:%s"
  :doc "Sets the name of chainsetup.
If not specified, defaults either to \"command-line-setup\" or to the file
name from which chainsetup was loaded.  Whitespaces are not allowed."
  :format "%t: %v%h"
  :pattern ".*"
  :tag "Chainsetup name")

(define-widget 'ecasound-buffer-size 'ecasound-cli-arg
  "A Custom Widget for the -b:buffer size argument."
  :arg-format "-b:%s"
  :doc "Sets the size of buffer in samples (must be an exponent of 2).
This is quite an important option. For real-time processing, you should set
this as low as possible to reduce the processing delay.  Some machines can
handle buffer values as low as 64 and 128.  In some circumstances (for
instance when using oscillator envelopes) small buffer sizes will make
envelopes act more smoothly.  When not processing in real-time (all inputs
and outputs are normal files), values between 512 - 4096 often give better
results."
  :format "%t: %v%h"
  :pattern "[0-9]+"
  :tag "Buffer size")

(define-widget 'ecasound-debug-level 'set
  "Custom widget for the -d:nnn argument."
  :arg-format "-d:%s"
  :args '((const :tag "Errors" 1)
	  (const :tag "Info" 2)
	  (const :tag "Subsystems" 4)
	  (const :tag "Module names" 8)
	  (const :tag "User objects" 16)
	  (const :tag "System objects" 32)
	  (const :tag "Functions" 64)
	  (const :tag "Continuous" 128)
	  (const :tag "EIAM return values" 256))
  :doc "Set the debug level"
  :match 'ecasound-cli-arg-match
  :pattern "[0-9]+"
  :string-match 'ecasound-cli-arg-string-match
  :tag "Debug level"
  :value-to-external
  (lambda (widget value)
    (format (widget-get widget :arg-format)
	    (number-to-string (apply #'+ (widget-apply widget :value-get)))))
  :value-to-internal
  (lambda (widget value)
    (when (widget-apply widget :string-match value)
      (let ((level (string-to-number (match-string 1 value)))
	    (levels (nreverse
		     (mapcar (lambda (elt) (car (last elt)))
			     (widget-get widget :args)))))
	(if (or (> level (apply #'+ levels)) (< level 0))
	    (error "Invalid debug level %d" level)
	  (delq nil
		(mapcar (lambda (elem)
			  (when (eq (/ level elem) 1)
			    (setq level (- level elem))
			    elem)) levels)))))))
 
(define-widget 'ecasound-args 'set
  ""
  :args '((const :tag "Start ecasound in interactive mode" "-c")
	  (const :tag "Print all debug information to stderr"
		 :doc "(unbuffered, plain output without ncurses)"
		 "-D")
	  (ecasound-debug-level)
	  (list :format "%v" :inline t
		(const :tag "Allow remote connections:" "--daemon")
		(ecasound-daemon-port :tag "Daemon port" "--daemon-port:2868"))
	  (ecasound-buffer-size "-b:1024")
	  (ecasound-chainsetup-name "-n:eca-el-setup")
	  (const :tag "Truncate outputs" :format "%t\n%h"
		     :doc "All output objects are opened in overwrite mode.
Any existing files will be truncated."
		     "-x")
	      (const :tag "Open outputs for updating"
		     :doc "Ecasound opens all outputs - if target format allows it - in readwrite mode."
		     "-X")
	      (repeat :tag "Others" :inline t (string :tag "Argument"))))

(defcustom ecasound-arguments '("-c" "-d:259" "--daemon" "--daemon-port:2868"
				"-n:eca-el-setup")
  "*Command line arguments used when starting an ecasound process."
  :group 'ecasound
  :type 'ecasound-args)

(defun ecasound-daemon-port ()
  "Return the port number defined in `ecasound-arguments'."
  (let ((elem (member* "^--daemon-port:\\(.*\\)" ecasound-arguments
		       :test #'string-match)))
    (if elem
	(match-string 1 (car elem)))))

(defun ecasound-customize-startup ()
  (interactive)
  (customize-variable 'ecasound-arguments))

(defcustom ecasound-program "ecasound"
  "*Ecasound's executable.
This program is executed when the user invokes \\[ecasound]."
  :group 'ecasound
  :type 'file)

(defcustom ecasound-prompt-regexp "^ecasound[^>]*> "
  "Regexp to use to match the prompt."
  :group 'ecasound
  :type 'regexp)

(defcustom ecasound-parse-cleanup-buffer t
  "*Indicates if `eci-output-filter' should cleanup the buffer.
This means the loglevel, msgsize and return type will get removed if
parsed successfully."
  :group 'ecasound
  :type 'boolean)

(defcustom ecasound-error-hook nil
  "*Called whenever a ECI error happens."
  :group 'ecasound
  :type 'hook)

(defcustom ecasound-message-hook '(ecasound-print-message)
  "*Hook called whenever a message except loglevel 256 (eci) is received.
Arguments are LOGLEVEL and STRING."
  :group 'ecasound
  :type 'hook)

(defun ecasound-print-message (level msg)
  "Simple function which prints every message regardless which loglevel."
  (message "Ecasound (%d): %s" level msg))

(defface ecasound-error-face '((t (:foreground "White" :background "Red")))
  "Face used to highlight errors."
  :group 'ecasound)

(make-variable-buffer-local
 (defvar ecasound-daemon nil
   "If initialized, this variable holds the buffer of a daemon channel."))

(make-variable-buffer-local
 (defvar ecasound-daemon-timer nil))

(defvar ecasound-chain-map nil
  "Keymap used for Chain operations.")
(define-prefix-command 'ecasound-chain-map)
(define-key 'ecasound-chain-map "a" 'eci-c-add)
(define-key 'ecasound-chain-map "c" 'eci-c-clear)
(define-key 'ecasound-chain-map "d" 'eci-c-deselect)
(define-key 'ecasound-chain-map "m" 'eci-c-mute)
(define-key 'ecasound-chain-map "x" 'eci-c-remove)
(define-key 'ecasound-chain-map (kbd "M-s") 'ecasound-cs-map)
(define-key 'ecasound-chain-map (kbd "M-o") 'ecasound-cop-map)
(defvar ecasound-cop-map nil
  "Keymap used for Chain operator operations.")
(define-prefix-command 'ecasound-cop-map)
(define-key 'ecasound-cop-map "a" 'eci-cop-add)
(define-key 'ecasound-cop-map "i" 'eci-cop-select)
(define-key 'ecasound-cop-map "l" 'eci-cop-list)
(define-key 'ecasound-cop-map "s" 'eci-cop-status)
(define-key 'ecasound-cop-map "x" 'eci-cop-remove)
(defvar ecasound-audioin-map nil
  "Keymap used for audio input objects.")
(define-prefix-command 'ecasound-audioin-map)
(define-key 'ecasound-audioin-map "a" 'eci-ai-add)
(define-key 'ecasound-audioin-map "f" 'eci-ai-forward)
(define-key 'ecasound-audioin-map "r" 'eci-ai-rewind)
(define-key 'ecasound-audioin-map "x" 'eci-ai-remove)
(defvar ecasound-audioout-map nil
  "Keymap used for audio output objects.")
(define-prefix-command 'ecasound-audioout-map)
(define-key 'ecasound-audioout-map "a" 'eci-ao-add)
(define-key 'ecasound-audioout-map "d" 'eci-ao-add-default)
(define-key 'ecasound-audioout-map "f" 'eci-ao-forward)
(define-key 'ecasound-audioout-map "r" 'eci-ao-rewind)
(define-key 'ecasound-audioout-map "x" 'eci-ao-remove)
(defvar ecasound-cs-map nil
  "Keymap used for Chainsetup operations.")
(define-prefix-command 'ecasound-cs-map)
(define-key 'ecasound-cs-map "a" 'eci-cs-add)
(define-key 'ecasound-cs-map "c" 'eci-cs-connect)
(define-key 'ecasound-cs-map "d" 'eci-cs-disconnect)
(define-key 'ecasound-cs-map "f" 'eci-cs-forward)
(define-key 'ecasound-cs-map "r" 'eci-cs-rewind)
(define-key 'ecasound-cs-map "t" 'eci-cs-toogle-loop)

(defvar ecasound-iam-mode-map
  (let ((map (make-sparse-keymap)))
    (set-keymap-parent map comint-mode-map)
    (define-key map "\t" 'pcomplete)
    (define-key map (kbd "M-c") 'ecasound-chain-map)
    (define-key map (kbd "M-i") 'ecasound-audioin-map)
    (define-key map (kbd "M-o") 'ecasound-audioout-map)
    (define-key map (kbd "M-\"") 'eci-command)
    map))

(easy-menu-define
  ecasound-iam-cs-menu ecasound-iam-mode-map
  "Chainsetup menu."
  (list "Chainsetup"
	["Add..." eci-cs-add t]
	["Load..." eci-cs-load t]
	["Save" eci-cs-save t]
	["Save As..." eci-cs-save-as t]
	["List" eci-cs-list t]
	["Select" eci-cs-select t]
	["Select via index" eci-cs-index-select t]
	"-"
	["Selected" eci-cs-selected t]
	["Valid?" eci-cs-is-valid t]
	["Connect" eci-cs-connect (eci-cs-is-valid)]
	["Disconnect" eci-cs-disconnect t]
	["Get position" eci-cs-get-position t]
	["Get length" eci-cs-get-length t]
	["Get length in samples" eci-cs-get-length-samples t]
	["Forward..." eci-cs-forward t]
	["Rewind..." eci-cs-rewind t]
	))
(easy-menu-add ecasound-iam-cs-menu ecasound-iam-mode-map)
(easy-menu-define
  ecasound-iam-c-menu ecasound-iam-mode-map
  "Chain menu."
  (list "Chain"
	["Add..." eci-c-add t]
	["Select..." eci-c-select t]
	["Select All" eci-c-select-all t]
	["Deselect..." eci-c-deselect (> (length (eci-c-selected)) 0)]
	["Selected" eci-c-selected t]
	["Mute" eci-c-mute t]
	["Clear" eci-c-clear t]
	))
(easy-menu-add ecasound-iam-c-menu ecasound-iam-mode-map)
(easy-menu-define
  ecasound-iam-cop-menu ecasound-iam-mode-map
  "Chain Operator menu."
  (list "ChainOp"
	["Add..." eci-cop-add (> (length (eci-c-selected)) 0)]
	["Select..." eci-cop-select t]
	["Edit..." ecasound-cop-edit t]
	"-"
	["Select parameter..." eci-copp-select t]
	["Get parameter value" eci-copp-get t]
	["Set parameter value..." eci-copp-set t]
	))
(easy-menu-add ecasound-iam-c-menu ecasound-iam-mode-map)
(easy-menu-define
  ecasound-iam-ai-menu ecasound-iam-mode-map
  "Audio Input Object menu."
  (list "AudioIn"
	["Add..." eci-ai-add (> (length (eci-c-selected)) 0)]
	["List" eci-ai-list t]
	["Select..." eci-ai-select t]
	["Index select..." eci-ai-index-select t]
	"-"
	["Attach" eci-ai-attach t]
	["Remove" eci-ai-remove t]
	["Forward..." eci-ai-forward t]
	["Rewind..." eci-ai-rewind t]
	))
(easy-menu-add ecasound-iam-ai-menu ecasound-iam-mode-map)
(easy-menu-define
  ecasound-iam-ao-menu ecasound-iam-mode-map
  "Audio Output Object menu."
  (list "AudioOut"
	["Add..." eci-ao-add (> (length (eci-c-selected)) 0)]
	["Add default" eci-ao-add-default (> (length (eci-c-selected)) 0)]
	["List" eci-ao-list t]
	["Select..." eci-ao-select t]
	["Index select..." eci-ao-index-select t]
	"-"
	["Attach" eci-ao-attach t]
	["Remove" eci-ao-remove t]
	["Forward..." eci-ao-forward t]
	["Rewind..." eci-ao-rewind t]
	))
(easy-menu-add ecasound-iam-ao-menu ecasound-iam-mode-map)

(easy-menu-define
  ecasound-menu global-map
  "Ecasound menu."
  (list "Ecasound"
	["Get session" ecasound t]
	"-"
	["Normalize..." ecasound-normalize t]
	["Signalview..." ecasound-signalview t]
	"-"
	["Customize startup..." ecasound-customize-startup t]
	))
(easy-menu-add ecasound-menu global-map)

(make-variable-buffer-local
 (defvar ecasound-mode-string nil))

(define-derived-mode ecasound-iam-mode comint-mode "EIAM"
  "Special mode for ecasound processes in interactive mode."
  (set (make-local-variable 'comint-prompt-regexp)
       (set (make-local-variable 'paragraph-start)
	    ecasound-prompt-regexp))
  (add-hook 'comint-output-filter-functions 'eci-output-filter nil t)
  (add-hook 'comint-input-filter-functions 'eci-input-filter nil t)
  (ecasound-iam-setup-pcomplete)
  (setq mode-line-format (copy-sequence mode-line-format))
  (when (not (featurep 'xemacs))
    (setcdr (setcdr mode-line-format (member 'mode-line-buffer-identification mode-line-format))
	    (member 'global-mode-string mode-line-format)))
  (setcar (member 'global-mode-string mode-line-format) 'ecasound-mode-string))

(defun ecasound-mode-line-cop-list (handle)
  (let ((list (eci-cop-list handle))
	(sel (1- (eci-cop-selected handle)))
	(str ""))
    (dotimes (i (length list) str)
      (setq str (format "%s%s%s%s"
			str
			(if (= i sel) "*" "")
			(nth i list)
			(if (= i (length list)) "" ","))))))

(defsubst ecasound-daemon-p ()
  (and (buffer-live-p ecasound-daemon)
       (eq (process-status ecasound-daemon) 'open)))

(defun ecasound-kill-timer ()
  (interactive)
  (when ecasound-daemon-timer
    (cancel-timer ecasound-daemon-timer)))

(defun ecasound-kill-daemon ()
  (interactive)
  (ecasound-kill-timer)
  (when (ecasound-daemon-p)
    (kill-buffer ecasound-daemon)))

(defun ecasound-update-mode-line (buffer)
  (when (and (buffer-live-p buffer)
	     (get-buffer-window buffer 'visible))
    (with-current-buffer buffer
      (when (ecasound-daemon-p)
	(setq ecasound-mode-string
	      (list
	       (eci-engine-status ecasound-daemon)
	       " [" (ecasound-position-to-string
		     (eci-cs-get-position ecasound-daemon))
	       "/" (ecasound-position-to-string
		     (eci-cs-get-length ecasound-daemon))
	       "]"
	       )
	      header-line-format
	      (list
	       (eci-cs-selected ecasound-daemon)
	       " [" (if (eci-cs-is-valid ecasound-daemon)
			"valid"
		      "N/A") "]: ("
	       (mapconcat 'identity (eci-c-list ecasound-daemon) ",")
	       ") "
	       (mapconcat 'identity (eci-c-selected ecasound-daemon) ",")))))))

(defun ecasound-setup-timer ()
  (when (ecasound-daemon-p)
    (setq ecasound-daemon-timer
	  (run-with-timer 1 1
			  'ecasound-update-mode-line
			  (current-buffer)))))

(make-variable-buffer-local
 (defvar eci-int-output-mode-wellformed-flag nil
   "Indicates if int-output-mode-wellformed was successfully initialized."))

;;;###autoload
(defun ecasound (&optional buffer)
  "Run an inferior ecasound, with I/O through BUFFER.
BUFFER defaults to `*ecasound*'.
Interactively, a prefix arg means to prompt for BUFFER.
If BUFFER exists but ecasound process is not running, make new ecasound
process using `ecasound-arguments'.
If BUFFER exists and ecasound process is running, just switch to BUFFER.
The buffer is put in ecasound mode, giving commands for sending input and
completing IAM commands.  See `ecasound-iam-mode'.

\(Type \\[describe-mode] in the ecasound buffer for a list of commands.)"
  (interactive
   (list
    (and current-prefix-arg
	 (read-buffer "Ecasound buffer: " "*ecasound*"))))
  (when (null buffer)
    (setq buffer "*ecasound*"))
  (if (not (comint-check-proc buffer))
      (pop-to-buffer
       (save-excursion
	 (set-buffer
	  (apply 'make-comint
		 "ecasound"
		 ecasound-program
		 nil
		 ecasound-arguments))
	 (ecasound-iam-mode)
	 ;; Flush process output
	 (while (accept-process-output
		 (get-buffer-process (current-buffer))
		 1))
	 (if (consp ecasound-program)
	     ;; If we're connecting via tcp/ip, we're most probably connecting
	     ;; to a daemon-mode ecasound session.
	     (setq comint-input-sender 'ecasound-network-send
		   eci-int-output-mode-wellformed-flag t)
	   (let ((eci-hide-output nil))
	     (if (not (eq (eci-command "int-output-mode-wellformed") t))
		 (message "Failed to initialize properly"))))
	 (when (member "--daemon" ecasound-arguments)
	   (ecasound-setup-daemon)
	   (when (and ecasound-daemon (eq (process-status ecasound-daemon) 'open))
	     (ecasound-setup-timer)))
	 (current-buffer)))
    (pop-to-buffer buffer)))

(defun ecasound-setup-daemon ()
  (setq ecasound-daemon
	(save-excursion
	  (set-buffer
	   (make-comint
	    "ecasound-daemon"
	    (cons "localhost" (ecasound-daemon-port))))
	  (ecasound-iam-mode)
	  (setq comint-input-sender 'ecasound-network-send
		eci-int-output-mode-wellformed-flag t)
	  (set (make-variable-buffer-local 'comint-highlight-prompt) nil)
	  (setq comint-output-filter-functions '(eci-output-filter))
	  (current-buffer)))
  (add-hook 'kill-buffer 'ecasound-kill-daemon nil t))

(defun ecasound-delete-last-in-and-output ()
  "Delete the region of text generated by the last in and output.
This is usually used to hide ECI requests from the user."
  (delete-region
   (save-excursion (goto-char comint-last-input-end) (forward-line -1)
		   (unless (looking-at ecasound-prompt-regexp)
		     (error "Assumed ecasound-prompt"))
		   (point))
   comint-last-output-start))

(make-variable-buffer-local
 (defvar eci-last-command nil
   "Last command sent to the ecasound process."))

(make-variable-buffer-local
 (defvar ecasound-last-parse-start nil
   "Where to start parsing if output is received.
This marker is advanced everytime a successful parse happens."))

(defun eci-input-filter (string)
  "Track commands sent to ecasound.
Argument STRING is the input sent."
  (when (string-match "^[[:space:]]*\\([a-zA-Z-]+\\)[\n\t ]+" string)
    (setq eci-last-command (match-string-no-properties 1 string)
	  ;; This is a precaution, but it makes sense
	  ecasound-last-parse-start (point))
    (when (or (string= eci-last-command "quit")
	      (string= eci-last-command "q"))
      ;; Prevents complete hangup, still a bit mysterius
      (ecasound-kill-daemon))))

(defun ecasound-network-send (proc string)
  "Function for sending to PROC input STRING via network."
  (comint-send-string proc string)
  (comint-send-string proc "\n"))

(defun eci-output-filter (string)
  "Parse and use ECI messages.
This function should be used on `comint-output-filter-functions' hook.
STRING is the string originally received and inserted into the buffer."
  (let ((start (or ecasound-last-parse-start (point-min)))
	(end (process-mark (get-buffer-process (current-buffer)))))
    (when (< start end)
      (save-excursion
	(let (type value (end (copy-marker end)))
	  (goto-char start)
	  (while (re-search-forward
		  "\\([0-9]\\{1,3\\}\\) \\([0-9]\\{1,5\\}\\)\\( \\(.*\\)\\)?\n"
		  end t)
	    (let* ((loglevel (string-to-number (match-string 1)))
		   (msgsize (string-to-number (match-string 2)))
		   (return-type (match-string-no-properties 4))
		   (msg (buffer-substring-no-properties
			 (point)
			 (progn
			   (if (> (- (point-max) (point)) msgsize)
			     (progn
			       (forward-char msgsize)
			       (if (not (save-match-data
					  (looking-at
					   "\\(\n\n\\|\n\n\\)")))
				   (error "Malformed ECI message")
				 (point)))
			     (point-max))))))
	      (when (= msgsize (length msg))
		(if (and (= loglevel 256)
			 (string= return-type "e"))
		    (add-text-properties
		     (match-end 0) (point)
		     (list 'face 'ecasound-error-face)))
		(when ecasound-parse-cleanup-buffer
		  (delete-region (match-beginning 0) (if (= msgsize 0)
							 (point)
						       (match-end 0)))
		  (delete-char 1))
		(setq ecasound-last-parse-start (point))
		(if (not (= loglevel 256))
		    (run-hook-with-args 'ecasound-message-hook loglevel msg)
		  (setq value msg
			type (if (string-match "\\(.*\\)" return-type)
				 (match-string 1 return-type)
			       return-type))))))
	  (when type
	    (setq eci-return-value value
		  eci-return-type type
		  eci-result
		  (cond
		   ((string= type "-")
		    (if (string= eci-last-command
				 "int-output-mode-wellformed")
			(setq eci-int-output-mode-wellformed-flag t)
		      t))
		   ((string= type "f")
		    (string-to-number value))
		   ((or (string= type "i")
			(string= type "li"))
		    (string-to-number value))
		   ((string= type "s")
		    (cond
		     ((string= eci-last-command "map-cop-list")
		      (setq eci-map-cop-list (eci-process-map-list value)))
		     ((string= eci-last-command "map-ladspa-list")
		      (setq eci-map-ladspa-list (eci-process-map-list value)))
		     ((string= eci-last-command "map-ctrl-list")
		      (setq eci-map-ctrl-list (eci-process-map-list value)))
		     ((string= eci-last-command "map-preset-list")
		      (setq eci-map-preset-list (eci-process-map-list value)))
		     ((string= eci-last-command "cop-status")
		      (eci-process-cop-status value))
		     (t value)))
		   ((string= type "S")
		    (if (string= eci-last-command "int-cmd-list")
			(setq ecasound-iam-commands (split-string value ","))
		      (split-string value ",")))
		   ((string= type "e")
		    (run-hook-with-args 'ecasound-error-hook value)
		    (list 'error value))
		   (t (error "Unimplemented return type %s" type))))))))))

(make-variable-buffer-local
 (defvar eci-return-type nil
   "The return type of the last received return value as a string."))

(make-variable-buffer-local
 (defvar eci-return-value nil
   "The last received return value as a string."))

(make-variable-buffer-local
 (defvar eci-result nil
   "The last received return value as a Lisp Object."))

(defmacro defeci (name &optional args doc &rest body)
  "Defines an ECI command.
Argument NAME is used for the function name with eci- as prefix.
Optional argument ARGS specifies the arguments this ECI command has.
Optional argument DOC is the docstring used for the defined function.
BODY can start with keyword arguments to indicated certain special cases. The following keyword arguments are
implemented:
 :cache VARNAME  The command should try to find a cached version of the result
                 in VARNAME."
  (let ((sym (intern (format "eci-%S" name)))
	(pcmpl-sym (intern (format "pcomplete/ecasound-iam-mode/%S" name)))
	(cmd `(eci-command
	       ,(if args
		    `(format ,(format "%S %s"
				      name (mapconcat #'caddr args " "))
			     ,@(mapcar
				(lambda (arg)
				  `(if (or (stringp ,(car arg))
					   (numberp ,(car arg)))
				       ,(car arg)
				     (mapconcat #'identity ,(car arg) ",")))
				args))
		  (format "%S" name))
	       buffer-or-process))
	cache cache-doc pcmpl aliases)
    (while (keywordp (car body))
      (case (pop body)
	(:cache (setq cache (pop body)))
	(:cache-doc (setq cache-doc (pop body)))
	(:pcomplete (setq pcmpl (pop body)))
	(:alias (setq aliases (pop body)))
	(t (pop body))))
    (when (and (not (eq aliases nil))
	       (not (consp aliases)))
      (setq aliases (list aliases)))
    `(progn
     ,(if cache
	  `(make-variable-buffer-local
	    (defvar ,cache ,@(if cache-doc (list nil cache-doc) (list nil)))))
     (defun ,sym
       ,(if args (append (mapcar #'car args) `(&optional buffer-or-process))
	  `(&optional buffer-or-process))
       ,(if doc doc "")
       ,(if args `(interactive
		   ,(if (let (done)
			  (mapcar (lambda (x) (when x (setq done t)))
				  (mapcar #'stringp (mapcar #'cadr args)))
			  done)
			(mapconcat #'identity (mapcar #'cadr args) "\n")
		      `(list ,@(mapcar #'cadr args))))
	  `(interactive))
       ,@(cond
	  ((and cache (eq body nil))
	   `((let ((cached (with-current-buffer
			       (ecasound-find-buffer buffer-or-process)
			     ,(or cache (and (ecasound-daemon-p)
					     (with-current-buffer
						 ecasound-daemon
					       ,cache))))))
	       (if cached
		   cached
		 ,cmd))))
	  ((eq body nil)
	   `(,cmd))
	  (t body)))
     ,@(mapcar
	(lambda (alias) `(defalias ',(intern (format "eci-%S" alias))
			   ',sym)) aliases)
     ,(when pcmpl
	`(progn
	   ,(if (and (eq pcmpl 'doc) (stringp doc) (not (string= doc "")))
		`(defun ,pcmpl-sym ()
		   (message ,doc)
		   (throw 'pcompleted t))
	      `(defun ,pcmpl-sym ()
		 ,pcmpl))
	   ,@(mapcar
	      (lambda (alias)
		`(defalias ',(intern (format "pcomplete/ecasound-iam-mode/%S" alias))
		   ',pcmpl-sym))
	      aliases))))))

(defeci map-cop-list ()
  ""
  :cache eci-map-cop-list
  :cache-doc "If non-nil, contains the chainop object map.
It has the form
 ((NAME PREFIX DESCR ((ARGNAME DESCR DEFAULT LOW HIGH TYPE) ...)) ...)

Use `eci-map-cop-list' to fill this list with data.")

(defeci map-ctrl-list ()
  ""
  :cache eci-map-ctrl-list
  :cache-doc "If non-nil, contains the chainop controller object map.
It has the form
 ((NAME PREFIX DESCR ((ARGNAME DESCR DEFAULT LOW HIGH TYPE) ...)) ...)

Use `eci-map-ctrl-list' to fill this list with data.")

(defeci map-ladspa-list ()
  ""
  :cache eci-map-ladspa-list
  :cache-doc "If non-nil, contains the LADSPA object map.
It has the form
 ((NAME PREFIX DESCR ((ARGNAME DESCR DEFAULT LOW HIGH TYPE) ...)) ...)

Use `eci-map-ladspa-list' to fill this list with data.")

(defeci map-preset-list ()
  ""
  :cache eci-map-preset-list
  :cache-doc "If non-nil, contains the preset object map.
It has the form
 ((NAME PREFIX DESCR ((ARGNAME DESCR DEFAULT LOW HIGH TYPE) ...)) ...)

Use `eci-map-preset-list' to fill this list with data.")

;;; Ecasound-iam-mode pcomplete functions

(defun ecasound-iam-setup-pcomplete ()
  "Setup buffer-local functions for pcomplete in `ecasound-iam-mode'."
  (set (make-local-variable 'pcomplete-command-completion-function)
       (lambda ()
	 (pcomplete-here (if ecasound-iam-commands
			     ecasound-iam-commands
			   (eci-hide-output eci-int-cmd-list)))))
  (set (make-local-variable 'pcomplete-command-name-function)
       (lambda ()
         (pcomplete-arg 'first)))
  (set (make-local-variable 'pcomplete-parse-arguments-function)
       'ecasound-iam-pcomplete-parse-arguments))

(defun ecasound-iam-pcomplete-parse-arguments ()
  "Parse arguments in the current region.
\" :,\" are considered for splitting."
  (let ((begin (save-excursion (comint-bol nil) (point)))
	(end (point))
	begins args)
    (save-excursion
      (goto-char begin)
      (while (< (point) end)
	(skip-chars-forward " \t\n,:")
	(setq begins (cons (point) begins))
	(let ((skip t))
	  (while skip
	    (skip-chars-forward "^ \t\n,:")
	    (if (eq (char-before) ?\\)
		(skip-chars-forward " \t\n,:")
	      (setq skip nil))))
	(setq args (cons (buffer-substring-no-properties
			  (car begins) (point))
			 args)))
      (cons (reverse args) (reverse begins)))))

(defun ecasound-input-file-or-device ()
  "Return a list of possible completions for input device name."
  (append (delq
	   nil
	   (mapcar
	    (lambda (elt)
	      (when (string-match
		     (concat "^" (regexp-quote pcomplete-stub)) elt)
		elt))
	    (list "alsa" "alsahw" "alsalb" "alsaplugin"
		  "arts" "loop" "null" "stdin")))
	  (pcomplete-entries)))

;;;; IAM commands

(defun eci-map-find-args (arg map)
  "Return the argument specification for ARG in MAP."
  (let (result)
    (while map
      (if (string= (nth 1 (car map)) arg)
	  (setq result (nthcdr 3 (car map))
		map nil)
	(setq map (cdr map))))
    result))

(defun ecasound-echo-arg (arg)
  "Display a chain operator parameter description from a eci-map-*-list
variable."
  (if arg
      (let ((type (nth 5 arg)))
	(message "%s%s%s, default %S%s%s"
		 (car arg)
		 (if type (format " (%S)" type) "")
		 (if (and (not (string= (nth 1 arg) ""))
			  (not (string= (car arg) (nth 1 arg))))
		     (format " (%s)" (nth 1 arg))
		   "")
		 (nth 2 arg)
		 (if (nth 4 arg) (format " min %S" (nth 4 arg)) "")
		 (if (nth 3 arg) (format " max %S" (nth 3 arg)) "")))
    (message "No help available")))


;;; ECI --- The Ecasound Control Interface

(defgroup eci nil
  "Ecasound Control Interface."
  :group 'ecasound)

(defcustom eci-program (or (getenv "ECASOUND") "ecasound")
  "*Program to invoke when doing `eci-init'."
  :group 'eci
  :type '(choice string (cons string string)))

(defcustom eci-arguments '("-c" "-D" "-d:256")
  "*Arguments used by `eci-init'."
  :group 'eci
  :type 'ecasound-args)

(defvar eci-hide-output nil
  "If non-nil, `eci-command' will remove the output generated.")

(defmacro eci-hide-output (&rest eci-call)
  `(if (ecasound-daemon-p)
       ,(append eci-call (list 'ecasound-daemon))
     (let ((eci-hide-output t))
       ,eci-call)))

(defun eci-init ()
  "Initialize a programmatic ECI session.
Every call to this function results in a new sub-process being created
according to `eci-program' and `eci-arguments'.  Returns the newly
created buffer.
The caller is responsible for terminating the subprocess at some point."
  (save-excursion
    (set-buffer
     (apply 'make-comint
	    "eci-ecasound"
	    eci-program
	    nil
	    eci-arguments))
    (ecasound-iam-mode)
    (while (accept-process-output (get-buffer-process (current-buffer)) 1))
    (if (eci-command "int-output-mode-wellformed")
	(current-buffer))))

(defun eci-interactive-startup ()
  "Used to interactively startup a ECI session using `eci-init'.
This will mostly be used for testing sessions and is equivalent
to `ecasound'."
  (interactive)
  (switch-to-buffer (eci-init)))

(defun ecasound-find-buffer (buffer-or-process)
  (cond
   ((bufferp buffer-or-process)
    buffer-or-process)
   ((processp buffer-or-process)
    (process-buffer buffer-or-process))
   ((and (eq major-mode 'ecasound-iam-mode)
	 (comint-check-proc (current-buffer)))
    (current-buffer))
   (t (error "Could not determine suitable ecasound buffer"))))

(defun eci-command (command &optional buffer-or-process)
  "Send a ECI command to a ECI host process.
COMMAND is the string to be sent, without a newline character.
If BUFFER-OR-PROCESS is nil, first look for a ecasound process in the current
buffer, then for a ecasound buffer with the name *ecasound*,
otherwise use the buffer or process supplied.
Return the string we received in reply to the command except
`eci-int-output-mode-wellformed-flag' is set, which means we can parse the
output via `eci-parse' and return a meaningful value."
  (interactive "sECI Command: ")
  (let* ((buf (ecasound-find-buffer buffer-or-process))
	 (proc (get-buffer-process buf)))
    (with-current-buffer buf
      (let ((moving (= (point) (point-max))))
	(setq eci-result 'waiting)
	(goto-char (process-mark proc))
	(insert command)
	(let (comint-eol-on-send)
	  (comint-send-input))
	(let ((here (point)) result)
	  (while (eq eci-result 'waiting)
	    (accept-process-output proc 0 30))
	  (setq result
		(if eci-int-output-mode-wellformed-flag
		    eci-result
		  ;; Backward compatibility.  Just return the string
		  (buffer-substring-no-properties here (save-excursion
					; Strange hack to avoid fields
							 (forward-char -1)
							 (beginning-of-line)
							 (if (not (= here (point)))
							     (forward-char -1))
							 (point)))))
	  (if moving (goto-char (point-max)))
	  (when (and eci-hide-output result)
	    (ecasound-delete-last-in-and-output))
	  result)))))

(defsubst eci-error-p ()
  "Predicate which can be used to check if the last command produced an error."
  (string= eci-return-type "e"))

;;; ECI commands implemented as lisp functions

(defeci int-cmd-list ()
  ""
  :cache ecasound-iam-commands
  :cache-doc "Available Ecasound IAM commands.")

(defeci run)

(defeci start)

(defeci cs-add ((chainsetup "sChainsetup to add: " "%s"))
  "Adds a new chainsetup with name `name`."
  :pcomplete doc)

(defeci cs-connect ()
  "Connect currently selected chainsetup to engine."
  :pcomplete doc)

(defeci cs-connected ()
  "Returns the name of currently connected chainsetup."
  :pcomplete doc)

(defeci cs-disconnect ()
  "Disconnect currently connected chainsetup."
  :pcomplete doc)

(defeci cs-forward
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer (format "Time in seconds to forward %s: "
			       (eci-hide-output eci-cs-selected)))) "%f")))

(defeci cs-get-length ()
  ""
  :alias get-length)

(defeci cs-get-length-samples ()
  ""
  :alias get-length-samples)

(defeci cs-get-position ()
  ""
  :alias (cs-getpos getpos get-position))

(defeci cs-index-select ((index "nChainsetup index: " "%d"))
  ""
  :alias cs-iselect)

(defeci cs-is-valid ()
  "Whether currently selected chainsetup is valid (=can be connected)?"
  :pcomplete doc
  (let ((val (eci-command "cs-is-valid" buffer-or-process)))
    (setq val 
	  (cond
	   ((= val 0)
	    nil)
	   ((= val 1)
	    t)
	   (t (error "Unexpected reply in cs-is-valid"))))
    (if (interactive-p)
	(message (format "Chainsetup is%s valid" (if val "" " not"))))
    val))

(defeci cs-list ()
  "Returns a list of all chainsetups."
  :pcomplete doc
  (let ((val (eci-command "cs-list" buffer-or-process)))
    (if (interactive-p)
	(message (concat "Available chainsetups: " 
			 (mapconcat #'identity val ", "))))
    val))

(defeci cs-load ((filename "fChainsetup filename: " "%s"))
  ""
  :pcomplete (pcomplete-here (pcomplete-entries)))

(defeci cs-remove ()
  "Removes currently selected chainsetup."
  :pcomplete doc)

(defeci cs-rewind
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer "Time in seconds to rewind chainsetup: ")) "%f"))
  "Rewinds the current chainsetup position by `time-in-seconds` seconds."
  :pcomplete doc
  :alias (rewind rw))

(defeci cs-save)

(defeci cs-save-as ((filename "FChainsetup filename: " "%s"))
  ""
  :pcomplete (pcomplete-here (pcomplete-entries)))

(defeci cs-selected ()
  "Returns the name of currently selected chainsetup."
  :pcomplete doc
  (let ((val (eci-command "cs-selected" buffer-or-process)))
    (if (interactive-p)
	(message (format "Selected chainsetup: %s" val)))
    val))

(defeci cs-status)

(defeci c-add ((chains "sChain(s) to add: " "%s"))
  "Adds a set of chains.  Added chains are automatically selected.
If argument CHAINS is a list, its elements are concatenated with ','.")

(defeci c-clear ()
  "Clear selected chains by removing all chain operators and controllers.
Doesn't change how chains are connected to inputs and outputs."
  :pcomplete doc)

(defun ecasound-read-list (prompt list)
  "Interactively prompt for a number of inputs until empty string.
PROMPT is used as prompt and LIST is a list of choices to choose from."
  (let ((avail list)
	result current)
    (while
	(and avail
	     (not
	      (string=
	       (setq current (completing-read prompt (mapcar #'list avail)))
	       "")))
      (setq result (cons current result)
	    avail (delete current avail)))
    (nreverse result)))

(defeci c-deselect
  ((chains (ecasound-read-list "Chain to deselect: " (eci-c-selected)) "%s"))
  "Deselects chains."
  :pcomplete (while (pcomplete-here (eci-c-selected))))

(defeci c-list)

(defeci c-mute ()
  "Toggle chain muting.  When chain is muted, all data that goes
through is muted."
  :pcomplete doc)

(defeci c-select ((chains (ecasound-read-list "Chain: " (eci-c-list)) "%s"))
  "Selects chains.  Other chains are automatically deselected."
  :pcomplete doc)

(defeci c-selected ()
  ""
  (let ((val (eci-command "c-selected" buffer-or-process)))
    (if (interactive-p)
	(if (null val)
	    (message "No selected chains")
	  (message (concat "Selected chains: "
			   (mapconcat #'identity val ", ")))))
    val))

(defeci c-select-all ()
  "Selects all chains."
  :pcomplete doc)

(defeci cs-select
  ((chainsetup
    (completing-read "Chainsetup: " (mapcar #'list (eci-cs-list)))
    "%s"))
  ""
  :pcomplete (pcomplete-here (eci-hide-output eci-cs-list)))

(defeci ai-add
  ((ifstring
    (let ((file (read-file-name "Input filename: ")))
      (if (file-exists-p file)
	  (expand-file-name file)
	file))
    "%s"))
  "Adds a new input object."
  :pcomplete (pcomplete-here (ecasound-input-file-or-device)))

(defeci ai-attach ()
  "Attaches the currently selected audio input object to all selected chains."
  :pcomplete doc)

(defeci ai-forward
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer (format "Time in seconds to forward %s: "
			       (eci-hide-output eci-ai-selected)))) "%f"))
  "Selected audio input object is forwarded by SECONDS.
Time should be given as a floating point value (eg. 0.001 is the same as 1ms)."
  :pcomplete doc
  :alias ai-fw)

(defeci ai-rewind
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer (format "Time in seconds to rewind %s: "
			       (eci-hide-output eci-ai-selected)))) "%f"))
  "Selected audio input object is rewinded by SECONDS.
Time should be given as a floating point value (eg. 0.001 is the same as 1ms)."
  :pcomplete doc
  :alias ai-rw)

(defeci ai-index-select ((index "nAudio Input index: " "%d"))
  "Select some audio input object based on a short index.
Especially file names can be rather long.  This command can be used to avoid
typing these long names when selecting audio objects.
INDEX is an integer value, where 1 refers to the first audio input.
You can use `eci-ai-list' to get a full list of currently available inputs."
  :pcomplete doc
  :alias ai-iselect)

(defeci ai-list)

(defeci ai-remove ()
  "Removes the currently selected audio input object from the chainsetup."
  :pcomplete doc)
(defeci ao-remove ()
  "Removes the currently selected audio output object from the chainsetup."
  :pcomplete doc)

(defeci ai-select ((name "sAudio Input Object name: " "%s"))
  "Selects an audio object.
NAME refers to the string used when creating the object.  Note! All input
object names are required to be unique.  Similarly all output names need to be
unique.  However, it's possible that the same object name exists both as an
input and as an output."
  :pcomplete (pcomplete-here (eci-hide-output eci-ai-list)))

(defeci ai-selected ()
  "Returns the name of the currently selected audio input object."
  :pcomplete doc)

(defeci ao-add ((filename "FOutput filename: " "%s"))
  ""
  :pcomplete (pcomplete-here (ecasound-input-file-or-device)))

(defeci ao-add-default)

(defeci ao-attach ()
  "Attaches the currently selected audio output object to all selected chains."
  :pcomplete doc)

(defeci ao-forward
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer (format "Time in seconds to forward %s: "
			       (eci-hide-output eci-ao-selected)))) "%f"))
  "Selected audio output object is forwarded by SECONDS.
Time should be given as a floating point value (eg. 0.001 is the same as 1ms)."
  :pcomplete doc
  :alias ao-fw)

(defeci ao-index-select ((index "nAudio Output index: " "%d"))
  "Select some audio output object based on a short index.
Especially file names can be rather long.  This command can be used to avoid
typing these long names when selecting audio objects.
INDEX is an integer value, where 1 refers to the first audio output.
You can use `eci-ao-list' to get a full list of currently available outputs."
  :pcomplete doc
  :alias ao-iselect)

(defeci ao-list)

(defeci ao-rewind
  ((seconds
    (if current-prefix-arg
	(prefix-numeric-value current-prefix-arg)
      (read-minibuffer (format "Time in seconds to rewind %s: "
			       (eci-hide-output eci-ai-selected)))) "%f"))
  "Selected audio output object is rewinded by SECONDS.
Time should be given as a floating point value (eg. 0.001 is the same as 1ms)."
  :pcomplete doc
  :alias ai-rw)

(defeci ao-select ((name "sAudio Output Object name: " "%s"))
  "Selects an audio object.
NAME refers to the string used when creating the object.  Note! All output
object names need to be unique.  However, it's possible that the same object
name exists both as an input and as an output."
  :pcomplete (pcomplete-here (eci-hide-output eci-ao-list)))

(defeci ao-selected ()
  "Returns the name of the currently selected audio output object."
  :pcomplete doc)

(defeci engine-status)

(defmacro ecasound-complete-cop-map (map)
  (let ((m (intern (format "eci-map-%S-list" map))))
    `(progn
       (cond
	((= pcomplete-last 2)
	 (pcomplete-next-arg)
	 (pcomplete-here
	  (sort (mapcar (lambda (elt) (nth 1 elt))
			(eci-hide-output ,m))
		#'string-lessp)))
	((> pcomplete-last 2)
	 (ecasound-echo-arg
	  (nth (- pcomplete-last 3)
	       (eci-map-find-args
		(pcomplete-arg -1) (eci-hide-output ,m)))))))))

(defeci cop-add
  ((string
    (if current-prefix-arg
	(read-string "Chainop to add: " "-")
      (let* ((cop
	      (completing-read
	       "Chain operator: "
	       (append (eci-hide-output eci-map-cop-list)
		       (eci-hide-output eci-map-ladspa-list)
		       (eci-hide-output eci-map-preset-list))))
	     (entry (or (assoc cop (eci-map-cop-list))
			(assoc cop (eci-map-ladspa-list))
			(assoc cop (eci-map-preset-list))))
	     (arg (nth 1 entry)))
	(concat
	 (cond
	  ((assoc cop (eci-map-cop-list))
	   (concat "-" arg ":"))
	  ((assoc cop (eci-map-ladspa-list))
	   (concat "-el:" arg ","))
	  ((assoc cop (eci-map-preset-list))
	   (concat "-pn:" arg ",")))
	 (mapconcat #'ecasound-read-copp (nthcdr 3 entry) ","))))
    "%s"))
  ""
  :pcomplete
  (progn
    (cond
     ((= pcomplete-last 1)
      (pcomplete-here
       (append
	'("-el:" "-pn:")
	(mapcar
	 (lambda (elt)
	   (concat "-" (nth 1 elt) ":"))
	 (eci-hide-output eci-map-cop-list)))))
     ((string= (pcomplete-arg) "-el")
      (ecasound-complete-cop-map ladspa))
     ((string= (pcomplete-arg) "-pn")
      (ecasound-complete-cop-map preset))
     ((> pcomplete-last 1)
      (ecasound-echo-arg
       (nth (- pcomplete-last 2)
	    (eci-map-find-args
	     (substring (pcomplete-arg) 1)
	     (eci-hide-output eci-map-cop-list))))))
    (throw 'pcompleted t)))

(defeci cop-list)

(defeci cop-remove)

(defeci cop-select
  ((index "nChainop to select: " "%d")))

(defeci cop-selected)

;; FIXME: Command seems to be broken in CVS.
(defeci cop-set ((cop "nChainop id: " "%d")
		 (copp "nParameter id: " "%d")
		 (value "nValue: " "%f"))
  "Changes the value of a single chain operator parameter.
Unlike other chain operator commands, this can also be used during processing."
  :pcomplete doc)


(defeci ctrl-add
  ((string
    (if current-prefix-arg
	(read-string "Controller to add: " "-")
      (let ((ctrl (assoc
		   (completing-read
		    "Chain operator controller controller: "
		    (eci-hide-output eci-map-ctrl-list))
		   (eci-hide-output eci-map-ctrl-list))))
	(concat "-" (nth 1 ctrl) ":"
		(mapconcat #'ecasound-read-copp (nthcdr 3 ctrl) ","))))
    "%s")))

(defeci ctrl-select
  ((index "nController to select: " "%d")))

(defeci copp-select
  ((index "nChainop parameter to select: " "%d")))

(defeci copp-get)

(defeci copp-set
  ((value "nValue for Chain operator parameter: " "%f")))

;;;; ECI Examples

(defun eci-example ()
  "Implements the example given in the ECI documentation."
  (interactive)
  (save-current-buffer
    (set-buffer (eci-init))
    (display-buffer (current-buffer))
    (eci-cs-add "play_chainsetup")
    (eci-c-add "1st_chain")
    (call-interactively #'eci-ai-add)
    (eci-ao-add "/dev/dsp")
    (eci-cop-add "-efl:100")
    (eci-cop-select 1) (eci-copp-select 1)
    (eci-cs-connect)
    (eci-command "start")
    (sit-for 1)
    (while (and (string= (eci-engine-status) "running")
		(< (eci-get-position) 15))
      (eci-copp-set (+ (eci-copp-get) 500))
      (sit-for 1))
    (eci-command "stop")
    (eci-cs-disconnect)
    (message (concat "Chain operator status: "
                      (eci-command "cop-status")))))

(defun eci-make-temp-file-name (suffix)
  (concat (make-temp-name
	   (expand-file-name "emacs-eci" temporary-file-directory))
	  suffix))

(defun ecasound-read-from-minibuffer (prompt default)
  (let ((result (read-from-minibuffer
		 (format "%s (default %S): " prompt default)
		 nil nil nil nil default)))
    (if (and result (not (string= result "")))
	result
      default)))

(defconst ecasound-signalview-clipped-threshold (- 1.0 (/ 1.0 16384)))

(defconst ecasound-signalview-bar-length 55)

(defun ecasound-position-to-string (secs)
  (format "%02d:%02d.%02d"
	  (/ secs 60)
	  (% (round (floor secs)) 60)
	  (* (- secs (floor secs)) 100)))

(defun ecasound-signalview (bufsize format input output)
  (interactive
   (list
    (ecasound-read-from-minibuffer "Buffersize" "128")
    (ecasound-read-from-minibuffer "Format" "s16_le,2,44100,i")
    (let ((file (read-file-name "Input: ")))
      (if (file-exists-p file)
	  (expand-file-name file)
	file))
    (ecasound-read-from-minibuffer "Output" "null")))
  (let* ((ecasound-parse-cleanup-buffer nil)
	 (handle (eci-init))
	 (channels (string-to-number (nth 1 (split-string format ","))))
	 (chinfo (make-vector channels nil)))
    (dotimes (ch channels) (aset chinfo ch (cons 0 0)))
    (eci-cs-add "signalview" handle)
    (eci-c-add "analysis" handle)
    (eci-cs-set-audio-format format handle)
    (eci-ai-add input handle)
    (eci-ao-add output handle)
    (eci-cop-add "-evp" handle)
    (eci-cop-add "-ev" handle)
    (set-buffer (get-buffer-create "*Ecasound-signalview*"))
    (erase-buffer)
    (dotimes (ch channels)
      (insert "---\n"))
    (setq header-line-format
	 (list (concat "Channel#"
		       (make-string (- ecasound-signalview-bar-length 3) 32)
		       "| max-value  clipped")))
    (set (make-variable-buffer-local 'ecasignalview-position) "unknown")
    (set (make-variable-buffer-local 'ecasignalview-engine-status) "unknown")
    (setq mode-line-format
	  (list
	   (list
	    (- ecasound-signalview-bar-length 3)
	    (format "Input: %s, output: %s" input output)
	    'ecasignalview-engine-status)
	   " | " 'ecasignalview-position))
    (switch-to-buffer-other-window (current-buffer))
    (eci-cs-connect handle)
    (eci-start handle)
    (sit-for 0.8)
    (eci-cop-select 1 handle)
    (while (string= (setq ecasignalview-engine-status
			  (eci-engine-status handle)) "running")
      (let ((inhibit-quit t) (inhibit-redisplay t))
	(setq ecasignalview-position
	      (ecasound-position-to-string (eci-cs-get-position handle)))
	(delete-region (point-min) (point-max))
	(dotimes (ch channels)
	  (insert (format "ch%d: " (1+ ch)))
	  (let ((val (progn (eci-copp-select (1+ ch) handle)
			    (eci-copp-get handle)))
		(bl ecasound-signalview-bar-length))
	    (insert
	     (concat
	      (make-string (round (* val bl)) ?*)
	      (make-string (- bl (round (* val bl))) ? )))
	    (if (> val (car (aref chinfo ch)))
		(setcar (aref chinfo ch) val))
	    (if (> val ecasound-signalview-clipped-threshold)
	      (incf (cdr (aref chinfo ch))))
	    (insert (format "| %.4f     %d\n" (car (aref chinfo ch))
			    (cdr (aref chinfo ch))))))
	(goto-char (point-min)))
      (sit-for 0.1)
      (fit-window-to-buffer))
    (goto-char (point-max))
    (let ((pos (point)))
      (insert
       (nth 2
	    (nth 2
		 (nthcdr 2
			 (assoc "Volume analysis"
				(assoc "analysis"
				       (eci-cop-status handle)))))))
      (goto-char pos))
    (recenter channels)
    (fit-window-to-buffer)))

(defun ecasound-normalize (filename)
  "Normalize a audio file using ECI."
  (interactive "fFile to normalize: ")
  (let ((tmpfile (eci-make-temp-file-name ".wav")))
    (unwind-protect
	(with-current-buffer (eci-init)
	  (display-buffer (current-buffer)) (sit-for 1)
	  (eci-cs-add "analyze") (eci-c-add "1")
	  (eci-ai-add filename) (eci-ao-add tmpfile)
	  (eci-cop-add "-ev")
	  (message "Analyzing sample data...")
	  (eci-cs-connect) (eci-run)
	  (eci-cop-select 1) (eci-copp-select 2)
	  (let ((gainfactor (eci-copp-get)))
	    (eci-cs-disconnect)
	    (if (<= gainfactor 1)
		(message "File already normalized!")
	      (eci-cs-add "apply") (eci-c-add "1")
	      (eci-ai-add tmpfile) (eci-ao-add filename)
	      (eci-cop-add "-ea:100")
	      (eci-cop-select 1)
	      (eci-copp-select 1)
	      (eci-copp-set (* gainfactor 100))
	      (eci-cs-connect) (eci-run) (eci-cs-disconnect)
	      (message "Done"))))
      (if (file-exists-p tmpfile)
	  (delete-file tmpfile)))))

;;; Utility functions for converting strings to data-structures.

(defvar eci-cop-status-header
  "### Chain operator status (chainsetup '\\([^']+\\)') ###\n")

(defun eci-process-cop-status (string)
  (with-temp-buffer
    (insert string) (goto-char (point-min))
    (when (re-search-forward eci-cop-status-header nil t)
      (let (result)
	(while (re-search-forward "Chain \"\\([^\"]+\\)\":\n" nil t)
	  (let ((c (match-string-no-properties 1)) chain)
	    (while (re-search-forward
		    "\t\\([0-9]+\\)\\. \\(.+\\): \\(.*\\)\n?" nil t)
	      (let ((n (string-to-number (match-string 1)))
		    (name (match-string-no-properties 2))
		    (args
		     (mapcar
		      (lambda (elt)
			(when (string-match
			       "\\[\\([0-9]+\\)\\] \\(.*\\) \\([0-9.-]+\\)$"
			       elt)
			  (list (match-string-no-properties 2 elt)
				(string-to-number (match-string 1 elt))
				(string-to-number (match-string 3 elt)))))
		      (split-string
		       (match-string-no-properties 3) ", "))))
		(if (looking-at "\tStatus info:\n")
		    (setq args
			  (append
			   args
			   (list
			    (list
			     "Status info" nil
			     (buffer-substring
			      (progn (forward-line 1) (point))
			      (or (re-search-forward "\n\n" nil t)
				  (point-max))))))))
		(setq chain (cons (append (list name n) args) chain))))
	    (setq result (cons (reverse (append chain (list c))) result))))
	result))))

(defun eci-process-map-list (string)
  "Parse the output of a map-xxx-list ECI command and return an alist.
STRING is the string returned by a map-xxx-list command."
  (mapcar
   (lambda (elt)
     (append
      (list (nth 1 elt) (nth 0 elt) (nth 2 elt))
      (let (res (count (string-to-number (nth 3 elt))))
	(setq elt (nthcdr 4 elt))
	(while (> count 0)
	  (setq
	   res
	   (cons
	    (list (nth 0 elt) (nth 1 elt)
		  (string-to-number (nth 2 elt)) ;; default value
		  (when (string= (nth 3 elt) "1")
		    (string-to-number (nth 4 elt)))
		  (when (string= (nth 5 elt) "1")
		    (string-to-number (nth 6 elt)))
		  (cond
		   ((string= (nth 7 elt) "1")
		    'toggle)
		   ((string= (nth 8 elt) "1")
		    'integer)
		   ((string= (nth 9 elt) "1")
		    'logarithmic)
		   ((string= (nth 10 elt) "1")
		    'output))) res)
	   elt (nthcdr 11 elt)
	   count (1- count)))
	(reverse res))))
   (mapcar (lambda (str) (split-string str ","))
	   (split-string string "\n"))))

(defeci cs-set-audio-format
  ((format (ecasound-read-from-minibuffer
	    "Audio format" "s16_le,2,44100,i") "%s")))

(defeci cop-register)
(defeci preset-register)
(defeci ctrl-register)

(defeci cop-status)

(defeci ladspa-register)

(defun ecasound-read-copp (copp)
  "Interactively read one chainop parameter."
  (let* ((completion-ignore-case t)
	 (default (format "%S" (nth 2 copp)))
	 (answer
	  (read-from-minibuffer
	   (concat
	    (car copp)
	    " (default " default "): ")
	   nil nil nil nil
	   default)))
    (if (and answer (not (string= answer "")))
	answer
      default)))

;;; ChainOp Editor

(defvar ecasound-cop-edit-mode-map
  (let ((map (make-keymap)))
    (set-keymap-parent map widget-keymap)
    map))

(define-derived-mode ecasound-cop-edit-mode fundamental-mode "COP-edit"
  "A major mode for editing ecasound chain operators.")

(defun ecasound-cop-edit ()
  "Edit the chain operator settings of the current session interactively.
This is done using the ecasound-cop widget."
  (interactive)
  (let ((cb (current-buffer))
	(chains (eci-cop-status)))
    (switch-to-buffer-other-window (generate-new-buffer "*cop-edit*"))
    (ecasound-cop-edit-mode)
    (mapc
     (lambda (chain)
       (widget-insert (format "Chain %s:\n" (car chain)))
       (mapc
	(lambda (cop)
	  (apply 'widget-create 'ecasound-cop :buffer cb cop))
	(cdr chain)))
     chains)
    (widget-setup)
    (goto-char (point-min))))

(define-widget 'ecasound-cop 'default
  "A Chain Operator.
:children is a list of ecasound-copp widgets."
  :convert-widget
  (lambda (widget)
    (let ((args (widget-get widget :args)))
      (when args
	(widget-put widget :tag (car args))
	(widget-put widget :cop-number (nth 1 args))
	(widget-put widget :args (cddr args))))
    widget)
  :value-create
  (lambda (widget)
    (widget-put
     widget :children
     (mapcar
      (lambda (copp-arg)
	(apply 'widget-create-child-and-convert
	     widget '(ecasound-copp) copp-arg))
      (widget-get widget :args))))
  :format-handler
  (lambda (widget escape)
    (cond
     ((eq escape ?i)
      (widget-put
       widget :cop-select
       (widget-create-child-value
	widget '(ecasound-cop-select) (widget-get widget :cop-number))))))
  :format "%i %t\n%v")

(define-widget 'ecasound-cop-select 'link
  "Select this chain operator parameter."
  :help-echo "RET to select."
  :button-prefix ""
  :button-suffix ""
  :format "%[%v.%]"
  :action
  (lambda (widget &rest ignore)
    (let ((buffer (widget-get (widget-get widget :parent) :buffer)))
      (eci-cop-select (widget-value widget) buffer))))

;;;; A Chain Operator Parameter Widget.

; This is used as a component of the cop widget.

(define-widget 'ecasound-copp 'number
  "A Chain operator parameter."
  :action 'ecasound-copp-action
  :convert-widget 'ecasound-copp-convert
  :format "  %i %v (%t)\n"
  :format-handler 'ecasound-copp-format-handler
  :size 10)

(defun ecasound-copp-convert (widget)
  "Convert args."
  (let ((args (widget-get widget :args)))
    (when args
      (widget-put widget :tag (car args))
      (widget-put widget :copp-number (nth 1 args))
      (widget-put widget :value (nth 2 args))
      (widget-put widget :args nil)))
  widget)

(defun ecasound-copp-format-handler (widget escape)
  (cond
   ((eq escape ?i)
    (widget-put
     widget
     :copp-select
     (widget-create-child-value
      widget
      '(ecasound-copp-select)
      (widget-get widget :copp-number))))
   ((eq escape ?s)
    (widget-put
     widget
     :slider
     (widget-create-child-value
      widget
      '(slider)
      (string-to-number (widget-get widget :value)))))))

(defun ecasound-copp-action (widget &rest ignore)
  "Sets WIDGETs value in its associated ecasound buffer."
  (let ((buffer (widget-get (widget-get widget :parent) :buffer)))
    (if (widget-apply widget :match (widget-value widget))
	(progn
	  (widget-apply (widget-get (widget-get widget :parent)
				    :cop-select) :action)
	  (widget-apply (widget-get widget :copp-select) :action)
	  (eci-copp-set (widget-value widget) buffer))
      (message "Invalid"))))

(defvar ecasound-copp-select-keymap
  (let ((map (copy-keymap widget-keymap)))
    (define-key map "+" 'ecasound-copp-increase)
    (define-key map "-" 'ecasound-copp-decrease)
    map)
  "Keymap used inside an copp.")

(defun ecasound-copp-increase (pos &optional event)
  (interactive "@d")
  ;; BUG, if we do this, the field is suddently no longer editable, why???
  (let ((widget (widget-get (widget-at pos) :parent)))
    (widget-value-set
     widget
     (+ (widget-value widget) 1))
    (widget-apply widget :action)
    (widget-setup)))

(defun ecasound-copp-decrease (pos &optional event)
  (interactive "@d")
  (let ((widget (widget-get (widget-at pos) :parent)))
    (widget-value-set
     widget
     (- (widget-value widget) 1))
    (widget-apply widget :action)
    (widget-setup)))

(define-widget 'ecasound-copp-select 'link
  "Select this chain operator parameter."
  :help-echo "RET to select, +/- to set in steps."
  :keymap ecasound-copp-select-keymap
  :format "%[%v%]"
  :action 'ecasound-copp-select-action)

(defun ecasound-copp-select-action (widget &rest ignore)
  "Selects WIDGET in its associated ecasound buffer."
  (let ((buffer (widget-get (widget-get (widget-get widget :parent) :parent)
			    :buffer)))
    (eci-copp-select (widget-get widget :value) buffer)))

(define-widget 'slider 'default
  "A slider."
  :action 'widget-slider-action
  :button-prefix ""
  :button-suffix ""
  :format "(%[%v%])"
  :keymap
  (let ((map (copy-keymap widget-keymap)))
    (define-key map "\C-m" 'widget-slider-press)
    (define-key map "+" 'widget-slider-increase)
    (define-key map "-" 'widget-slider-decrease)
    map)
  :value-create 'widget-slider-value-create
  :value-delete 'ignore
  :value-get 'widget-value-value-get
  :size 70
  :value 0)

(defun widget-slider-press (pos &optional event)
  "Invoke slider at POS."
  (interactive "@d")
  (let ((button (get-char-property pos 'button)))
    (if button
	(widget-apply-action
	 (widget-value-set
	  button
	  (- pos (overlay-start (widget-get button :button-overlay))))
	 event)
      (let ((command (lookup-key widget-global-map (this-command-keys))))
        (when (commandp command)
          (call-interactively command))))))

(defun widget-slider-increase (pos &optional event)
  "Increase slider at POS."
  (interactive "@d")
  (widget-slider-change pos #'+ 1 event))

(defun widget-slider-decrease (pos &optional event)
  "Decrease slider at POS."
  (interactive "@d")
  (widget-slider-change pos #'- 1 event))

(defun widget-slider-change (pos function value &optional event)
  "Change slider at POS by applying FUNCTION to old-value and VALUE."
  (let ((button (get-char-property pos 'button)))
    (if button
	(widget-apply-action
	 (widget-value-set button (apply function (widget-value button) value))
	 event)
      (let ((command (lookup-key widget-global-map (this-command-keys))))
        (when (commandp command)
          (call-interactively command))))))

(defun widget-slider-action (widget &rest ignore)
  "Set the current :parent value to :value."
  (widget-value-set (widget-get widget :parent)
		    (widget-value widget)))

(defun widget-slider-value-create (widget)
  "Create a sliders value."
  (let ((size (widget-get widget :size))
        (value (string-to-int (format "%.0f" (widget-get widget :value))))
        (from (point)))
    (insert-char ?\  value)
    (insert-char ?\| 1)
    (insert-char ?\  (- size value 1))))


;;; Ecasound .ewf major mode

(defgroup ecasound-ewf nil
  "Ecasound .ewf file mode related variables and faces."
  :prefix "ecasound-ewf-"
  :group 'ecasound)

(defcustom ecasound-ewf-output-device "/dev/dsp"
  "*Default output device used for playing .ewf files."
  :group 'ecasound-ewf
  :type 'string)

(defface ecasound-ewf-keyword-face '((t (:foreground "IndianRed")))
  "The face used for highlighting keywords."
  :group 'ecasound-ewf)

(defface ecasound-ewf-time-face '((t (:foreground "Cyan")))
  "The face used for highlighting time information."
  :group 'ecasound-ewf)

(defface ecasound-ewf-file-face '((t (:foreground "Green")))
  "The face used for highlighting the filname."
  :group 'ecasound-ewf)

(defface ecasound-ewf-boolean-face '((t (:foreground "Orange")))
  "The face used for highlighting boolean values."
  :group 'ecasound-ewf)

(defvar ecasound-ewf-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\t" 'pcomplete)
    (define-key map "\C-c\C-p" 'ecasound-ewf-play)
    map)
  "Keymap for `ecasound-ewf-mode'.")

(defvar ecasound-ewf-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?# "<" st)
    (modify-syntax-entry ?\n ">" st)
    st)
  "Syntax table for `ecasound-ewf-mode'.")

(defvar ecasound-ewf-font-lock-keywords
  '(("^\\s-*\\(source\\)[^=]+=\\s-*\\(.*\\)$"
     (1 'ecasound-ewf-keyword-face)
     (2 'ecasound-ewf-file-face))
    ("^\\s-*\\(offset\\)[^=]+=\\s-*\\([0-9.]+\\)$"
     (1 'ecasound-ewf-keyword-face)
     (2 'ecasound-ewf-time-face))
    ("^\\s-*\\(start-position\\)[^=]+=\\s-*\\([0-9.]+\\)$"
     (1 'ecasound-ewf-keyword-face)
     (2 'ecasound-ewf-time-face))
    ("^\\s-*\\(length\\)[^=]+=\\s-*\\([0-9.]+\\)$"
     (1 'ecasound-ewf-keyword-face)
     (2 'ecasound-ewf-time-face))
    ("^\\s-*\\(looping\\)[^=]+=\\s-*\\(true\\|false\\)$"
     (1 'ecasound-ewf-keyword-face)
     (2 'ecasound-ewf-boolean-face)))
  "Keyword highlighting specification for `ecasound-ewf-mode'.")

;;;###autoload
(define-derived-mode ecasound-ewf-mode fundamental-mode "EWF"
  "A major mode for editing ecasound .ewf files."
  (set (make-local-variable 'comment-start) "# ")
  (set (make-local-variable 'comment-start-skip) "#+\\s-*")
  (set (make-local-variable 'font-lock-defaults)
       '(ecasound-ewf-font-lock-keywords))
  (ecasound-ewf-setup-pcomplete))

;;; .ewf-mode pcomplete support

(defun ecasound-ewf-keyword-completion-function ()
  (pcomplete-here
   (list "source" "offset" "start-position" "length" "looping")))

(defun pcomplete/ecasound-ewf-mode/source ()
  (pcomplete-here (pcomplete-entries)))

(defun pcomplete/ecasound-ewf-mode/offset ()
  (message "insert audio object at offset (seconds) [read,write]")
  (throw 'pcompleted t))

(defun pcomplete/ecasound-ewf-mode/start-position ()
  (message "start offset inside audio object (seconds) [read]")
  (throw 'pcompleted t))

(defun pcomplete/ecasound-ewf-mode/length ()
  (message "how much of audio object data is used (seconds) [read]")
  (throw 'pcompleted t))

(defun pcomplete/ecasound-ewf-mode/looping ()
  (pcomplete-here (list "true" "false")))

(defun ecasound-ewf-parse-arguments ()
  "Parse whitespace separated arguments in the current region."
  (let ((begin (save-excursion (beginning-of-line) (point)))
	(end (point))
	begins args)
    (save-excursion
      (goto-char begin)
      (while (< (point) end)
	(skip-chars-forward " \t\n=")
	(setq begins (cons (point) begins))
	(let ((skip t))
	  (while skip
	    (skip-chars-forward "^ \t\n=")
	    (if (eq (char-before) ?\\)
		(skip-chars-forward " \t\n=")
	      (setq skip nil))))
	(setq args (cons (buffer-substring-no-properties
			  (car begins) (point))
			 args)))
      (cons (reverse args) (reverse begins)))))

(defun ecasound-ewf-setup-pcomplete ()
  (set (make-local-variable 'pcomplete-parse-arguments-function)
       'ecasound-ewf-parse-arguments)
  (set (make-local-variable 'pcomplete-command-completion-function)
       'ecasound-ewf-keyword-completion-function)
  (set (make-local-variable 'pcomplete-command-name-function)
       (lambda ()
	 (pcomplete-arg 'first)))
  (set (make-local-variable 'pcomplete-arg-quote-list)
       (list ? )))

;;; Interactive commands

;; FIXME: Make it use ECI.
(defun ecasound-ewf-play ()
  (interactive)
  (let ((ecasound-arguments (list "-c"
				  "-i" buffer-file-name
				  "-o" ecasound-ewf-output-device)))
    (and (buffer-modified-p)
	 (y-or-n-p "Save file before playing? ")
	 (save-buffer))
    (ecasound "*Ecasound-ewf Player*")))

(add-to-list 'auto-mode-alist (cons "\\.ewf$" 'ecasound-ewf-mode))

;; Local variables:
;; mode: outline-minor
;; outline-regexp: ";;;;* \\|"
;; End:

(provide 'ecasound)

;;; ecasound.el ends here

