(defvar howitworks-database (make-hash-table))

(defmacro howitworks (funcname help)
  (setf (gethash funcname howitworks-database) help)
  (values))

(defun usage (funcname details)
  (labels ((param-names
            (accum details-cdr)
            (cond
             ((endp details-cdr) accum)
             ((eql (first (first details-cdr)) 'param)
              (let ((param-name (second (first details-cdr))))
                (param-names (cons param-name accum) (rest details-cdr))))
             (t (param-names accum (rest details-cdr))))))
    (cons funcname (nreverse (param-names '() details)))))

(defun howitworks-query (funcname)
  (let ((details (gethash funcname howitworks-database)))
    (dolist (detail details (values))
      (case (first detail)
        (summary
         (format t "Usage: ~a~%~a~%" (usage funcname details) (rest detail)))
        (param
         (format t "~%~a:~%~a~%" (second detail) (nthcdr 2 detail)))
        (pre
         (format t "~%Precondition:~%~a~%" (rest detail)))
        (ret
         (format t "~%Returns:~%~a~%" (rest detail)))
        (effects
         (format t "~%Side effects:~%~a~%" (rest detail)))
        (otherwise
         (format t "~%~a:~%~a~%" (first detail) (rest detail)))))))

(howitworks unclear-fun
            ((summary This function does something.
                      This function does something.
                      This function does something.
                      This function does something.
                      This function does something.
                      This function does something.)
             (param A the first argument)
             (param B the second argument)
             (param C the third argument)
             (param D the fourth argument)
             (pre you understand this function)
             (ret the string "Hello")
             (effects none)))
(defun unclear-fun (A B C D)
  "Hello")
