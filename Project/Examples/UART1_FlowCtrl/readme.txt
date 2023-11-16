

   IMPORTANT: Only uart1 support hardware flow control.
   If you want to enable this option, please in project_config.h define
   
   #define SUPPORT_UART1_FLOWCNTL             1

   
1. connect uart1 tx to uart1 rx

2. CTS (pin21, or pin 15) to connect ground...
   then uart1 will keep to send data ...
   In this example, it use GPIO21 for CTSN.
     
   if you do NOT　connect CTS to ground... the transfer will stop.
   
   
3. you can also check RTS... it will toggle. 

   Because we call function uart_set_modem_status(1, (test_count &1));
   in timer isr function.
   
   In this example, it use GPIO20 for RTSN.
   
   According to hardware flow control protocol, the otherside see RTSN high, it should stop to send data.
   
4. if you connect RTS to CTS... then you will see the dma start/stop interval.... 
   there is gap for data transfer.
   