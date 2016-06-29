/*
 * author:charlee
 */
#ifndef _XMLPRINTERHTML5_H_
#define _XMLPRINTERHTML5_H_

#include "tinyxml2/tinyxml2.h"
#include "assert.h"
#include "stdio.h"

using namespace tinyxml2;
using namespace std;

class   XMLPrinterHTML5 : public XMLPrinter {
public:
    XMLPrinterHTML5 (FILE* file=0, bool compact = false, int depth = 0) :
        XMLPrinter (file, compact, depth)
    {}

protected:
    virtual void CloseElement () {
        if (_elementJustOpened && !isVoidElement (_stack.PeekTop())) {
            SealElementIfJustOpened();
            }
        XMLPrinter::CloseElement();
    }

    virtual bool isVoidElement (const char *name) {
        // Complete list of all HTML5 "void elements",
        // http://dev.w3.org/html5/markup/syntax.html
        static const char *list[] = {
            "area", "base", "br", "col", "command", "embed", "hr", "img",
            "input", "keygen", "link", "meta", "param", "source", "track", "wbr",
            NULL
        };

        // I could use 'bsearch', but I don't have MSVC to test on (it would work with gcc/libc).
        for (const char **p = list; *p; ++p) {
            if (!strcasecmp (name, *p)) {
                return true;
            }
        }

        return false;
    }
};

#endif
