#include <stdio.h>


int main(int argc, char* argv[])
{
    char buff[256];

    strcpy(buff, "[30;47m");
    char* str = buff;

    size_t len = strlen(str);

    if (len > 0 && str[0] == '[' && str[len - 1] == 'm')
    {
        str[len - 1] = '\0';
        str++;
        len -= 2;
        char* sep;
        if (sep = strchr(str, ';'))
        {
            int back = atoi(sep+1);
            *sep = '\0';
            int fore = atoi(str);

            printf("sep %s str %s\n", sep, str);

            printf("f = %d bk = %d\n", fore, back);
            //dsp_set_text_attr(_translate_fg_colour(fore), _translate_bk_colour(back));
            
            return 0;
        }

    }
    printf("failed\n");
	return 0;
}