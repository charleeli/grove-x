/*
 * author:charlee
 */
#ifndef _JSONHELPER_H_
#define _JSONHELPER_H_

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "assert.h"
#include "stdio.h"

using namespace rapidjson;
using namespace std;

class JsonHelper {
private:
    const char* filename_;
    
public:
    Document Root;

public:
    JsonHelper(const char* filename){
        filename_ = filename;
        Parse();
    }

private:    
    void Parse(){
        char *json_;
        size_t length_;
        
        FILE* fp = NULL;
        fp = fopen(filename_, "rb");
        if (fp) {
            assert("can't open json file!");
        }
         
        fseek(fp, 0, SEEK_END);
        length_ = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        json_ = (char*)malloc(length_ + 1);
        size_t readLength = fread(json_, 1, length_, fp);
        json_[readLength] = '\0';
        fclose(fp);

        Root.Parse(json_);
        
        free(json_);
        json_ = NULL;
    }
};

#endif

/*
int main()
{
    JsonHelper jh("./test.json");
    printf("%s\n",jh.Root["RedisAdmin"]["host"].GetString());
    printf("%d\n",jh.Root["RedisAdmin"]["port"].GetInt());
    
    return 0;
}
*/

