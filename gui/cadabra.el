(defun cadabra-insertion-filter (proc string)
  (with-current-buffer (process-buffer proc)
    (let ((moving (= (point) (process-mark proc))))
      (save-excursion
        ;; Insert the text, advancing the process marker.
        (goto-char (process-mark proc))
        (insert string)
        (set-marker (process-mark proc) (point)))
      (if moving (goto-char (process-mark proc))))))

(setq cdbproc (start-process "cadabra" "cadabra" "cadabra"))
(process-send-string cdbproc "@algorithms;\n")


\begin{cdb}
W_{a b c d} W_{e d e g};
@canonicalise!(%);
% W_{a b c d} W_{e d e g};
\end{cdb}