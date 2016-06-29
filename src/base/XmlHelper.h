/*
 * author:charlee
 */
#ifndef _XMLHELPER_H_
#define _XMLHELPER_H_

#include "tinyxml2/tinyxml2.h"
#include "assert.h"
#include "stdio.h"

using namespace tinyxml2;
using namespace std;

class XmlHelper{
private:
    const char* filename_;
    XMLDocument* doc_;

public:
    XmlHelper(const char* filename){
        filename_ = filename;
        doc_ = new XMLDocument();
        doc_->LoadFile( filename );
    }

    XMLDocument* getXMLDocument(){
        return this->doc_->ToDocument();
    }

    ~XmlHelper(){
        filename_ = "";
        delete doc_;
    }
};

#endif

