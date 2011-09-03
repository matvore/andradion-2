(require "howitworks")
(require "datafile")

(howitworks
 ncopies
 ((summary Supplies a list of any length of one particular element.)
  (param WHAT the element to place N copies of in the list.)
  (param N the number of times WHAT appears in the list.)
  (pre N is a non-negative integer.)
  (ret N copies of WHAT.)))  
(defun ncopies (WHAT N)
  (labels ((F (WHAT N L)
              (if (> N 0)
                  (F WHAT (1- N) (cons WHAT L))
                L)))
    (F WHAT N '())))

(howitworks
 xfer
 ((summary Transfers any number of a particular type of data pieces
           from the input stream (ASCII-formatted) to the output
           stream (binary-formatted.) Supplying a COUNT parameter
           repeats this process. The process is as follows...
           (((1) Read an integer N from the ASCII-formatted input stream
             IN. Integers are separated by whitespace and are in
             base 10.)
            ((2) Truncate N so its bit length fits in the number of bits
             indicated by TYPE.)
            ((3) Write N to OUT in little-endian.))
           For example... read "\"50\"" from IN. TYPE is
           byte--meaning N should be truncated to eight bits. Because
           N is 50--no truncation is necessary.
           Write hex value 32 to the output stream.)
  (param IN the input stream from which to read the number N.)
  (param OUT the output stream to write the binary representation of
         N.)
  (param TYPE one of (byte word dword))
  (param COUNT if non-nil--indicates the number of times to perform
         the operation. If nil--the operation is performed once.)
  (pre COUNT is a non-negative integer.)
  (ret nil if COUNT is 0--or the last values of N if (> COUNT 1)--or
       the only value of N if COUNT is nil or 1.)
  (effects (IN is some number of bytes forward--depending on how many
               bytes are consumed by the number itself. The number is read
               with the READ function.)
           (OUT is the number of bytes forward indicated by TYPE and
                N has been written to it.))))            
(defun xfer (IN OUT TYPE &optional COUNT)
  (cond
   ((not COUNT)
    (xfer IN OUT TYPE 1))

   ((not (zerop COUNT))
    (dotimes (i (1- COUNT) (df-write (read IN) TYPE OUT))
      (df-write (read IN) TYPE OUT)))
   
   (t nil)))

(defun xfer-palette (IN OUT)
  (let ((p-size (xfer IN OUT 'byte)))
    (xfer IN OUT 'byte (* (if (zerop p-size) 256 p-size) 3))))

(howitworks
 levelize-all
 ((summary Invokes levelize on all level IDs currently in the
           Andradion game.)
  (pre Each level has its preliminary level data available and valid in the
       "levels" directory.)
  (ret no return values.)
  (effects Each level has its final level data available and valid in
           the "Resource" directory.)))
(defun levelize-all ()
  (mapcar #'levelize
          '("1_" "2a" "2b" "3a" "3b" "4_" "5_" "6a" "6b" "7a" "7b"))
  (values))

(howitworks
 levelize
 ((summary This function is for Andradion 2 development of levels.
           It translates the ASCII data of a level to a binary
           layout. For example--if a palette entry in the ASCII
           file is listed "\"255 255 128\"" then Levelize will write
           the hex bytes (ff ff 80) to the output file.)
  (param LVL the two-character string that indicates which level to
         levelize. Must be two characters long.)
  (pre The file "levels\\??.dat" exists and has correctly-formatted
       data. ?? refers to the characters in LVL.)
  (ret no return values.)
  (effects The converted level data is found at "Resource\\??.lvl")))
(defun levelize (LVL)
  (let ((in
         (open (concatenate 'string "C:\\Andradion 2\\levels\\" LVL ".dat")
               :direction :input))
        (out
         (open (concatenate 'string "C:\\Andradion 2\\Resource\\" LVL ".lvl")
               :direction :output
               :element-type 'unsigned-byte)))
    (format t "~&Working on ~a:" LVL)
    (format t "~% -- palette")
    (xfer-palette in out)
    (format t "~% -- player location, weather script")
    (mapcar #'xfer (ncopies in 5) (ncopies out 5) '(word word byte word word))
    
    (let ((rect-count (xfer in out 'byte)))
      (format t "~% -- ~a indoor rectangles" rect-count)
      (xfer in out 'word (* 4 rect-count)))
    
    (let ((end-count (xfer in out 'byte)))
      (format t "~% -- ~a level ends" end-count)
      (dotimes (i end-count nil)
        (mapcar #'xfer (ncopies in 3) (ncopies out 3) '(byte word word))))

    (format t "~% -- enemies, powerups~%")
    (dotimes (i 7 nil)
      (xfer in out 'word (* 2 (xfer in out 'byte))))

    (close in)
    (close out)
    (values)))
