#ifndef STRINGBUILDER
#define STRINGBUILDER


#include <string.h>
#include <cstdlib>

#define   BF_SZ     4096


class StringBuilder
{
public:

    StringBuilder()
    {
        memset(__buf,0,BF_SZ+1);
        __ptr = __buf;
    }

    //添加指定长度的字符
    StringBuilder & Add(char *str, int sz = 32)
    {
        int len = strlen(str);
        len = ( len > sz ? sz:len);
        if ( __ptr -  __buf >= BF_SZ || __ptr + len -__buf>= BF_SZ)
            return *this;

        memcpy(__ptr,str,len);

        __ptr += len;

        return *this;
    }



    StringBuilder & operator << ( int value)
    {
        char buf[18] = {0};
        int len = 0;

        sprintf(buf,"%d",value);
        len = strlen(buf);

       if ( __ptr -  __buf >= BF_SZ || __ptr + len -__buf>= BF_SZ)
            return *this;

        memcpy(__ptr,buf,len);
        __ptr += len;
        return *this;
    }

    StringBuilder & operator << ( unsigned int value)
    {
        char buf[18] = {0};
        int len = 0;

        sprintf(buf,"%u",value);
        len = strlen(buf);

       if ( __ptr -  __buf >= BF_SZ || __ptr + len -__buf>= BF_SZ)
            return *this;

        memcpy(__ptr,buf,len);
        __ptr += len;
        return *this;
    }


    StringBuilder & operator << ( float value)
    {
        char buf[18] = {0};
        int len = 0;

        sprintf(buf,"%.6f",value);
        len = strlen(buf);

        if ( __ptr -  __buf >= BF_SZ || __ptr + len -__buf>= BF_SZ)
            return *this;

        memcpy(__ptr,buf,len);
        __ptr += len;
        return *this;
    }

    StringBuilder & operator << ( const char *str)
    {
        int len = strlen( str );

        if ( __ptr -  __buf >= BF_SZ || __ptr + len -__buf>= BF_SZ)
            return *this;

        memcpy(__ptr,str,len);
        __ptr += len;
        return *this;
    }

    ///
    /// \brief Clear
    ///
    void Clear()
    {
        memset(__buf,0,BF_SZ+1);
        __ptr = __buf;
    }

    ///
    /// \brief Data
    /// \return
    ///
   char * Data()
   {
       return __buf;
   }
   ///
   /// \brief str
   /// \return
   ///
   char *str()
   {
       return __buf;
   }

   void str( char * s)
   {
       Clear();
       Add(s, strlen(s));
   }

private:
    char        __buf[BF_SZ + 1];
    char        *__ptr;
};

#endif // STRINGBUILDER

