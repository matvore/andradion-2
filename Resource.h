class Resource {
protected:
  HGLOBAL data_handle;
  const BYTE *ptr;
  int size;

public:
  class Exception : public std::exception {
  public:
    const std::string message;

    Exception(const std::string& message) throw() : message(message) {}
    virtual ~Exception() throw() {}

    virtual const char *what() const throw() {return message.c_str();}
  };

  Resource(const char *resource_type, const char *resource_name)
    throw(Exception);
  ~Resource() throw();

  const BYTE *GetPtr(int offset = 0) throw() {return ptr + offset;}
  int GetSize() throw() {return size;}
};
