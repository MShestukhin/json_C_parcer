#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define close 0
#define open 1

#define false 0
#define true 1

#define array_object 2

#define HASH_MUL 31
#define HASH_SIZE 128

unsigned int hash(char * s  ){
    unsigned int h=0;
    char * p;
    for(p=s;*p!='\n';p++){
        h=h* HASH_MUL + (unsigned int)*p;
    }
    return h % HASH_SIZE;
}

struct json_object{
    int strings;
    int numbers;
    int objects;
    int arrays;
    int bools;
    int nulls;
    int level;
    int type;
    char* all_used_name[128];
    int all_used_types;
    struct json_object* next;
    int is_open;
};

void handl_error(struct json_object * json_obj, int err_code){
    static struct json_object * kernel_json_object;
    struct json_object* iterator;
    struct json_object* pointer_for_del;
    switch (err_code) {
    case 0:
        kernel_json_object=json_obj;
        break;
    case 1:
        iterator = kernel_json_object;
        while (iterator != NULL) {
            pointer_for_del = iterator;
            iterator= iterator->next;
            for(int i=0; i<128; i++)
                if(pointer_for_del->all_used_name[i] !=0) free(pointer_for_del->all_used_name[i]);
            free(pointer_for_del);
        }
        exit(1);
    }
}

struct json_object * new_json(){
    struct json_object* obj = malloc(sizeof(struct json_object));
    obj->next=NULL;
    obj->all_used_types=0;
    obj->arrays=0;
    obj->is_open=open;
    obj->bools=0;
    obj->strings=0;
    obj->numbers=0;
    obj->nulls=0;
    obj->level=0;
    obj->type=0;
    memset(obj->all_used_name, 0, sizeof(obj->all_used_name));
    return obj;
}


void pars_object(struct json_object * json_obj, FILE * fn, const char *c){
    char *ch= c;
    struct json_object* json_item;
    struct json_object* iterator;
    while (*ch !='}' && *ch !=EOF){
        *ch=fgetc(fn);
        switch (*ch) {
        case '\t':
            break;
        case '\"':
            pars_item(json_obj, fn,ch);
            break;
        case '{':
            iterator = json_obj;
            while (iterator->next != NULL) {
                iterator= iterator->next;
            }
            json_item = new_json();
            iterator->next=json_item;
            json_item->next=NULL;
            json_item->level=json_obj->level+1;
            pars_object(json_item,fn,ch);
            break;
        case '\n':
            break;
        case '\r':
            break;
        case ' ':
            break;
        default:
            printf("Not valid file : object\n");
            handl_error(json_obj,1);
        }
    }
    if(*ch == '}'){
        json_obj->is_open=close;
    }
    else {
        printf("Not valid file : object not close\n");
        handl_error(json_obj,1);
    }
}

void pars_array(struct json_object * json_obj, FILE * fn, const char *c){
    char *ch= c;
    while (*ch !=']' && *ch !=EOF){
        *ch=fgetc(fn);
        if(*ch == ']') break;
        switch (*ch) {
        case '\t':
            break;
        case '\n':
            break;
        case '\r':
            break;
        case '\"':
            pars_value(json_obj, fn,ch);
            break;
        case ',':
            continue;
        case '{':
            json_obj->type=array_object;
            pars_value(json_obj, fn,ch);
            break;
        case ' ':
            break;
        default:
            printf("Not valid file : array\n");
            handl_error(json_obj,1);
        }
    }
}

void pars_item(struct json_object * json_obj, FILE * fn, const char *c){
    char *ch= c;

    const int BUF_LEN = 50;

    int len = BUF_LEN;
    int i = 0;
    char * str=(char*)malloc(BUF_LEN*sizeof(char));
    *ch=fgetc(fn);
    while (*ch !='\"' && *ch !=EOF){
        if(i==len)
            str=(char*)realloc(str, len += BUF_LEN);
        str[i] = *ch;
        i++;
        *ch=fgetc(fn);
    }
    str[i] = '\n';
    str=(char*)realloc(str, sizeof(str));
    int key_hash=hash(str);
    if(json_obj->all_used_name[key_hash] == 0){
        json_obj->all_used_name[key_hash] = str;
    }
    else {
        if(strcmp(str, json_obj->all_used_name[key_hash]) != 0){
            //TODO json items as list node, for resolve problems with colisions
        }
        else {
            printf("Name is already used : %s\n",json_obj->all_used_name[key_hash]);
            handl_error(json_obj,1);
        }
    }
    while (*ch !=':' && *ch !=EOF){
        *ch=fgetc(fn);
    }
    *ch=fgetc(fn);
    pars_value(json_obj, fn,ch);
}

void pars_value(struct json_object * json_obj, FILE * fn, const char *c){
    char * ch= c;
    struct json_object* json_item;
    struct json_object* iterator;
    while (*ch !=',' && *ch !=EOF){
        switch (*ch) {
            case ' ': {
                break;
            }
            case '\n': {
                break;
            }
            case '\t': {
                break;
            }
            case '\r': {
                break;
            }
            case 'n': {
                char * false_str = "ull";
                while(*false_str != '\0'){
                   if((*false_str++) != (*ch=fgetc(fn))){  printf("Spelling error in null value"); handl_error(json_obj,1);}
                }
                if(json_obj->nulls == 0) {json_obj->nulls = true; json_obj->all_used_types++;}
                break;
            }
            case 'f': {
                char * false_str = "alse";
                while(*false_str != '\0'){
                   if((*false_str++) != (*ch=fgetc(fn))){  printf("Spelling error in false value"); handl_error(json_obj,1);}
                }
                if(json_obj->bools == 0) {json_obj->bools = true; json_obj->all_used_types++;}
                break;
            }
            case 't': {
                char * false_str = "rue";
                while(*false_str != '\0'){
                   if((*false_str++) != (*ch=fgetc(fn))){  printf("Spelling error in true value"); handl_error(json_obj,1);}
                }
                if(json_obj->bools == 0) {json_obj->bools = true; json_obj->all_used_types++;}
                break;
            }
            case '\"':
                *ch=fgetc(fn);
                while (*ch !='\"' && *ch !=EOF){
                    *ch=fgetc(fn);
                }
                if(json_obj->strings == 0) {json_obj->strings = true; json_obj->all_used_types++;}
                break;
            case '[':
                pars_array(json_obj,fn,ch);
                if(json_obj->arrays == 0) {json_obj->arrays = true; json_obj->all_used_types++;}
                break;
            case '{':
                    iterator = json_obj;
                    while (iterator->next != NULL) {
                        iterator= iterator->next;
                    }
                    json_item = new_json();
                    iterator->next=json_item;
                    json_item->next=NULL;
                    if(json_obj->type == array_object){
                        json_item->level=0;
                    }
                    else {
                        json_item->level=json_obj->level+1;
                    }
                    pars_object(json_item,fn,ch);
                    if(json_obj->objects == 0) {json_obj->objects = true; json_obj->all_used_types++;}
                break;
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if(json_obj->numbers == 0) {json_obj->numbers = true; json_obj->all_used_types++;}
                break;
            default:
                printf("Not valid file : item\n");
                handl_error(json_obj,1);
            }
            *ch=fgetc(fn);
            if (*ch == '}'){
                 pars_object(json_obj,fn,ch);
                break;
            }
            if (*ch == ']'){
                 pars_array(json_obj,fn,ch);
                break;
            }
    }
}

int main(int argc, char* argv[])
{
    FILE * fn;
    fn= fopen(argv[1],"r");
    if (fn == NULL) {
      printf("Error opening %s\n",argv[1]);
      return 1;
    }

    char *ch;
    struct json_object* kernel_json_object = new_json();
    kernel_json_object->next=NULL;
    kernel_json_object->level=0;
    handl_error(kernel_json_object,0);
    if((*ch=fgetc(fn)) !=EOF) {
        switch (*ch) {
        case '{':
            pars_object(kernel_json_object, fn,ch);
            break;
        case '[':
            pars_array(kernel_json_object,fn,ch);
            break;
        case '\"':
            pars_item(kernel_json_object,fn,ch);
            break;
        default:
            printf("Not valid json, syntax error\n");
            return 1;
        }
    }

    struct json_object* iterator = kernel_json_object;
    int most_includive_level=0;
    while (iterator != NULL) {
        if(iterator->level > most_includive_level) most_includive_level = iterator->level;
        if(iterator->is_open == close)
            printf("level : %d, all used types : %d\n",iterator->level, iterator->all_used_types);
        else
            printf("Not valid json\n");
        struct json_object* pointer_for_del = iterator;
        iterator= iterator->next;
        for(int i=0; i<128; i++)
            if(pointer_for_del->all_used_name[i] !=0) free(pointer_for_del->all_used_name[i]);
        free(pointer_for_del);
    }
    printf("Most included level object : %d\n",most_includive_level);

    fclose(fn);
    return 0;
}
