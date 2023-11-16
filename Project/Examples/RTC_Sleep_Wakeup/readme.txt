   
     This demo code shows the setting for how to let RT58x enter sleep mode and
   how to use RTC to wakeup the device at alarm time.
   
     In this example, when RT58x boot the first time, it will wait user to enter
   sleep mode in uart console, like tera term. 
   
     In uart console you will see some message like:
   
HELLO RTC sleep wakeup test
Cold boot
current time is  0- 1- 1  0: 0: 0
Please select mode you want to test:
0: For CPU sleep  RF run
1: For CPU sleep  RF sleep
2: For CPU sleep  RF deep sleep   


      You can press 0 for LOW_POWER_LEVEL_SLEEP0 /  1 for LOW_POWER_LEVEL_SLEEP1 / 2 for LOW_POWER_LEVEL_SLEEP0

      The default code waiting alarm time is "every mintues",. that is MM:00
  
      Because cortex-m3 will NOT response any ICE command when it sleeps.
   During this period, user can not use ICE to program new firmware.
   User can use 
     1. ISP download tool to download new firmware.   
     2. Switch the EVK board jump to force ISP mode, and program new firmware.
      

  
     