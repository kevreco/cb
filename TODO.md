Remove cb_CXFLAGS and only handle those:

CFLAGS: Extra flags to give to the C compiler.
CXXFLAGS: Extra flags to give to the C++ compiler.
CPPFLAGS: Extra flags to give to the C preprocessor and programs that use it (the C and Fortran compilers). 

with cb_CFLAGS etc.


- rename incrementabl build olfder like "ib_cache"
- rename all file like XXX.cache or 
cache-XXX
- Remove unncessary "CB_API" from some tests.
- Remove graveyard extension.
- add cb_dump

- create cb_add_many() if it does not exist already.


Create #ifdef _WIN32
LPSTR GetLastErrorAsString(void)
{
    // https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror

    DWORD errorMessageId = GetLastError();
    assert(errorMessageId != 0);

    LPSTR messageBuffer = NULL;

    DWORD size =
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // DWORD   dwFlags,
            NULL, // LPCVOID lpSource,
            errorMessageId, // DWORD   dwMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // DWORD   dwLanguageId,
            (LPSTR) &messageBuffer, // LPTSTR  lpBuffer,
            0, // DWORD   nSize,
            NULL // va_list *Arguments
        );

    return messageBuffer;
}