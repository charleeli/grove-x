#include "XMLPrinterHTML5.h"
#include <iostream>

using namespace tinyxml2;
using namespace std;

int main (void) {
    static const char input[] =
    "<html><body><p style='a'></p><br/>&copy;<col a='1' b='2'/><div a='1'></div></body></html>";

    XMLDocument doc (false);
    doc.Parse (input);

    std::cout << "INPUT:\n" << input << "\n\n";

    XMLPrinterHTML5 html5 (NULL, true);
    doc.Print (&html5);
    std::cout << "XMLPrinterHTML5:\n" << html5.CStr() << "\n";

    XMLPrinter prn (NULL, true);
    doc.Print (&prn);
    std::cout << "XMLPrinter (not valid HTML5):\n" << prn.CStr() << "\n\n";

    return 0;
}
