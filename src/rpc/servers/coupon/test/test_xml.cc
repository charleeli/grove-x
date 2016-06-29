#include "XmlHelper.h"
#include <iostream>

using namespace tinyxml2;
using namespace std;

int main (void) {
    XmlHelper xh("../../rpc/servers/coupon/test/utf8test.xml");
    if(0 != xh.getXMLDocument()->ErrorID()){
        cout<<"ErrorID:"<<xh.getXMLDocument()->ErrorID()<<endl;
        return -1;
    }

    XMLElement* element1 = xh.getXMLDocument()->FirstChildElement( "document" );
    XMLElement* element2 = element1->FirstChildElement( "SimplifiedChinese" );

    const char* title = element2->GetText();
    cout<<"title:"<<title<<endl;

    return 0;
}
