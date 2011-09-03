(require "howitworks")

(howitworks
 df-read-byte
 ((summary Reads a byte from the given input stream.)
  (param STREAM the stream from which to read.)
  (ret an integer B where (<= 0 B 255) that indicates the value of
       the byte read.)
  (effects the position of STREAM is one byte forward.)))
(defun df-read-byte (STREAM)
  (read-byte STREAM))

(howitworks
 df-read-word
 ((summary Reads a word (two bytes) from the given input stream and
           gives the resulting number as if the bytes were read
           little-endian.)
  (param STREAM the stream from which to read.)
  (ret an integer B where (<= 0 B 65535) that indicates the value of
       the word read.)
  (effects the position of STREAM is two bytes forward.)))
(defun df-read-word (STREAM)
  (let ((first-byte (df-read-byte STREAM)))
    (let ((second-byte (df-read-byte STREAM)))
      (+ first-byte (* 256 second-byte)))))
      
(howitworks
 df-read-dword
 ((summary Reads a double word (four bytes) from the given input
           stream and gives the resulting number as if the bytes were
           read little-endian.)
  (param STREAM the stream from which to read.)
  (ret an integer B where (<= 0 B (1- (expt 2 32))) that indicates the
       value of the double word read.)
  (effects the position of STREAM is four bytes forward.)))
(defun df-read-dword (STREAM)
  (let ((first-word (df-read-word STREAM)))
    (let ((second-word (df-read-word STREAM)))
      (+ first-word (* 65536 second-word)))))

(howitworks
 df-write-byte
 ((summary Writes a byte to the given output stream.)
  (param B the byte to write. Only the eight
         least significant bits are the ones written--so any integer
         is valid.)
  (param STREAM the stream to which to write.)
  (ret (logand B 255))
  (effects the position of STREAM is one byte forward.)))
(defun df-write-byte (B STREAM)
  (setq B (logand B 255))
  (write-byte B STREAM)
  B)

(howitworks
 df-write-word
 ((summary Writes a word (two bytes) to the given output stream.)
  (param W the word to write. Only the sixteen least significant bits
         are the ones written--so any integer is valid.)
  (param STREAM the stream to which to write.)
  (ret (logand W 65535))
  (effects the position of STREAM is two bytes forward.)))
(defun df-write-word (W STREAM)
  (df-write-byte W STREAM)
  (df-write-byte (floor W 256) STREAM)
  (logand W 65535))

(howitworks
 df-write-dword
 ((summary Writes a double word (four bytes) to the given output
           stream.) 
  (param DW the double word to write. Only the thirty-two least
         significant bits are the ones written--so any integer is
         valid.)
  (param STREAM the stream to which to write.)
  (ret (logand DW (1- (expt 2 32))))
  (effects the position of STREAM is four bytes forward.)))
(defun df-write-dword (DW STREAM)
  (df-write-word DW STREAM)
  (df-write-word (floor DW 65536) STREAM)
  (logand DW (1- (expt 2 32))))

(howitworks
 df-write
 ((summary Writes a piece of data of the given size to the given
           stream. The supported sizes are (byte word dword)
           For more information--see the following
           ((howitworks-query 'df-write-byte)
            (howitworks-query 'df-write-word)
            (howitworks-query 'df-write-dword)))
  (param DATA an integer to write. Any integer is valid.)
  (param TYPE a symbol indicating the type to write. Each possible
         value causes a different function to be invoked.
         ((the symbol byte causes df-write-byte to be called.)
          (the symbol word causes df-write-word to be called.)
          (the symbol dword causes df-write-dword to be called.)))
  (param STREAM the stream to which to write.)
  (ret see (howitworks-query 'df-write-???) where ??? is the symbol
       TYPE)  
  (effects see (howitworks-query 'df-write-???) where ??? is the
           symbol TYPE)))
(defun df-write (DATA TYPE STREAM)
  (case TYPE 
    ((byte) (df-write-byte DATA STREAM))
    ((word) (df-write-word DATA STREAM))
    ((dword) (df-write-dword DATA STREAM))))


