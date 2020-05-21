#include <stdio.h>
#include <malloc.h>

#define close 0
#define open 1

struct json_object{
    int strings;
    int numbers;
    int objects;
    int arrays;
    int bools;
    int nulls;
    int level;
    struct json_object* next;
    int is_open;
};

struct json_object * new_json(){
    struct json_object* obj = malloc(sizeof(struct json_object));
    obj->next=NULL;
    obj->arrays=0;
    obj->is_open=open;
    obj->bools=0;
    obj->strings=0;
    obj->numbers=0;
    obj->nulls=0;
    return obj;
}


void pars_object(struct json_object * json_obj, FILE * fn, const char c){
    char ch= c;
    while (ch !='\"' && ch !=EOF){
        ch=fgetc(fn);
        switch (ch) {
        case '\t':
            break;
        case '\"':
            pars_item(json_obj, fn,c);
            break;
        case '\n':
            break;
        case '\r':
            break;
        case ' ':
            break;
        case '}':
            printf("Not valid file\n");
            printf("Empty object\n");
            return 1;
        default:
            printf("Not valid file\n");
            return 1;
        }
    }
    return 0;
}

void pars_array(struct json_object * json_obj, FILE * fn, const char c){
    char ch= c;
    while (ch !='\"' && ch !=EOF){
        ch=fgetc(fn);
        switch (ch) {
        case '\t':
            break;
        case '\n':
            break;
        case '\r':
            break;
        case ' ':
            break;
        case ']':
            printf("Not valid file\n");
            printf("Empty array\n");
            return 1;
        default:
            printf("Not valid file\n");
            return 1;
        }
    }
    return 0;
}

void pars_item(struct json_object * json_obj, FILE * fn, const char c){
    char ch= c;
    while (ch !='\"' && ch !=EOF){
        ch=fgetc(fn);
    }
    while (ch !=':' && ch !=EOF){
        ch=fgetc(fn);
    }
    ch=fgetc(fn);
    pars_value(json_obj, fn,ch);
    return 0;
}

void pars_value(struct json_object * json_obj, FILE * fn, const char c){
    char ch= c;
    struct json_object* json_item;
    while (ch !=',' && ch !=EOF){
        switch (ch) {
            case ' ': {
                break;
            }
            case '\n': {
                break;
            }
            case 'n': {
                break;
            }
            case 'f': {
                break;
            }
            case 't': {
                break;
            }
            case '\"':
                ch=fgetc(fn);
                while (ch !='\"' && ch !=EOF){
                    ch=fgetc(fn);
                }
                json_obj->strings++;
                break;
            case '[':
                pars_array(json_obj,fn,ch);
                break;
            case '{':
                json_item = new_json();
                json_obj->next=json_item;
                json_item->next=NULL;
                json_item->level=json_obj->level+1;
                pars_object(json_item,fn,ch);
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
                json_obj->numbers++;
                break;
            default:
                printf("%c", ch);
                printf("Not valid file\n");
                return 1;
            }
            ch=fgetc(fn);
            if (ch == '}'){
                json_obj->is_open=close;
                break;
            }
    }

}

int main()
{
    FILE * fn;
    fn= fopen("file.txt","r");
    if (fn == NULL) {
      perror("Ошибка при открытии \"мойфайл\"");
      return 1;
    }

    char ch;
    struct json_object* kernel_json_object = new_json();
    kernel_json_object->next=NULL;
    kernel_json_object->level=0;
    while((ch=fgetc(fn)) !=EOF) {
        switch (ch) {
        case '{':
            pars_object(kernel_json_object, fn,ch);
            printf("%c", ch);
            break;
        case '[':
            pars_array(kernel_json_object,fn,ch);
            break;
        case '\"':
            pars_item(kernel_json_object,fn,ch);
            break;
//        default:
//            return 0;
        }
    }

    struct json_object* iterator = kernel_json_object;
    while (iterator != NULL) {
        if(!iterator->is_open)
            printf("level : %d, strings : %d, numbers : %d\n",iterator->level, iterator->strings, iterator->numbers);
        else
            printf("Not valid json");
        struct json_object* pointer_for_del = iterator;
        iterator= iterator->next;
        free(pointer_for_del);
    }

    fclose(fn);
    return 0;
}
