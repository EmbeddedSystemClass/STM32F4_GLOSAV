#ifndef FIFO__H
#define FIFO__H
/*
Идея использования DMA при обмене через UART в следующем:
Прием настраивается в кольцевом режиме в приемный буфер и идет аппаратно средствами DMA.
Для считывания из буфера предусмотрены функции проврки наличия информации в буфере и выборки 
(для этого используется программный указатель вершины считывания). 
Прерывания по приему не предусмотрены. Необходимо периодически опрашивать наличие и вычитывать принятую
информацию.

При передаче используется 2 буфера. Один для буферизации данных при передаче, второй для выдачи данных по DMA.
Буферы меняются местами по мере выдачи данных. Старт передачи иницируется либо сразу при записи в буфер
(если передача не активна), либо вручную (например по таймеру) для реализации накопления данных. Режим выбирается
макросом START_SEND_IMMEDIATE.
По завершении выдачи из буфера, производится проверка наличия данных для выдачи из второго буфера и запускается выдача.

Перед использованием данного драйвера необходимо зарезервировать место для буферов и инициализировать структуры данных FIFO,
запустить прием.
Также необходимо реализовать обработчик функции HAL_UART_TxCpltCallback (завершена выдача данных) и при необходимости обработчик
ошибок приема / передачи
Если отключен макро START_SEND_IMMEDIATE, то необходимо реализовать периодическую инициацию отправки данных.
*/


#define START_SEND_IMMEDIATE
#define USE_CALLBACK

typedef struct {
	unsigned char *pbuf;
	unsigned char size;
//	unsigned char tail;
	unsigned char head;
	UART_HandleTypeDef *uartHandle;
} rxfifo_t;

//typedef	void (*pBeforeTransmitCallback_func)(void);
//typedef	void (*pEndTransmitCallback_func)(void);

typedef struct {
	unsigned char *pbuf1;
	unsigned char *pbuf2;
	unsigned char size;
	unsigned char tail;
	unsigned char *pbuf;	// указатель на активный буфер для накопления данных
	UART_HandleTypeDef *uartHandle;
#ifdef USE_CALLBACK
	void (*pBeforeTransmitCallback)(void);
	void (*pEndTransmitCallback)(void);
#endif
} txfifo_t;

void rxFifoInit(UART_HandleTypeDef *uartHandle, rxfifo_t *pFifo, uint8_t* pBuf, uint8_t size);
void txFifoInit(UART_HandleTypeDef *uartHandle, txfifo_t *pFifo, uint8_t* pBuf1, uint8_t* pBuf2, uint8_t size);
int rxFifoStart(rxfifo_t *pFifo);
void rxFifoClear (rxfifo_t *pFifo);
uint8_t rxFifoBytesToRead (rxfifo_t *pFifo);
int16_t rxFifoPeekByte(rxfifo_t *pFifo);
int16_t rxFifoReadByte (rxfifo_t *pFifo);
uint8_t rxFifoReadBytes (rxfifo_t *pFifo, uint8_t *pBuffer, uint8_t bufLen);
uint32_t rxFifoReadBytesUntil (rxfifo_t *pFifo, uint8_t* pBuffer, uint8_t bufLen, uint8_t delimiter);
int txFifoStart(txfifo_t *pFifo);
void txFifoClear (txfifo_t *pFifo);
int txFifoPutChar(txfifo_t *pFifo, uint8_t c);
int txFifoPutStr(txfifo_t *pFifo, uint8_t* pc, uint8_t cnt);
uint8_t txFifoFreeSpace (txfifo_t *pFifo);
void txFifoCpltCallback (txfifo_t *pFifo);


#endif
