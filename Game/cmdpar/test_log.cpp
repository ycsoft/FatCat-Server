	#include <stdio.h>  
    #include "log.h"  
      
    int main(void)  
    {  
		Logger::GetLogger()->log_open("mycat");  
		int i = 10;
		float j = 23.45;
		Logger::GetLogger()->Info("info,%d,%f", i,j);
		Logger::GetLogger()->Error("error");
		Logger::GetLogger()->Warning("warn");
		Logger::GetLogger()->log_close();  
        return 0;  
    }  
