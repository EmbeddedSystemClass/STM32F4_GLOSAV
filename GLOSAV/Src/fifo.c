/*
Модуль FIFO для приема и передачи данных UART в режиме DMA
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "fifo.h"

void rxFifoInit(UART_HandleTypeDef *uartHandle, rxfifo_t *pFifo, uint8_t* pBuf, uint8_t size)
{
	assert_param(pFifo);
	assert_param(pBuf);
	assert_param(uartHandle);
	pFifo->pbuf=pBuf;
	pFifo->size=size;
	pFifo->head=0;
	pFifo->uartHandle=uartHandle;
}

void txFifoInit(UART_HandleTypeDef *uartHandle, txfifo_t *pFifo, uint8_t* pBuf1, uint8_t* pBuf2, uint8_t size)
{
	assert_param(pFifo);
	assert_param(pBuf1);
	assert_param(pBuf2);
	assert_param(uartHandle);
	pFifo->pbuf1=pBuf1;
	pFifo->pbuf2=pBuf2;
	pFifo->size=size;
	pFifo->tail=0;
	pFifo->pbuf=pBuf1;
	pFifo->uartHandle=uartHandle;
#ifdef USE_CALLBACK
	pFifo->pBeforeTransmitCallback = NULL;
	pFifo->pEndTransmitCallback = NULL;
#endif
}

/* Запуск приема по UART через DMA. Предварительно UART & DMA должны быть связаны в функции настройки */
int rxFifoStart(rxfifo_t *pFifo)
{
	assert_param(pFifo);
	if(HAL_UART_Receive_DMA(pFifo->uartHandle, (uint8_t*)pFifo->pbuf, pFifo->size) != HAL_OK)
		return -1;
	pFifo->head=0;
	return 0;
}


//////////////////////////// Модифицированные //////////////////////////////////////
void rxFifoClear (rxfifo_t *pFifo) {
	assert_param(pFifo);
	pFifo->head = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
}

/*
Количество принятых и не считанных байтов
*/
uint8_t rxFifoBytesToRead (rxfifo_t *pFifo) {
	assert_param(pFifo);
  uint8_t tail = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
 
    if (tail < pFifo->head) {
        return (pFifo->size) - (pFifo->head - tail);
    } else {
        return (tail - pFifo->head);
    }
}
 
/*
"посмотреть" байт для считывания (не берем)
-1 - empty
*/
int16_t rxFifoPeekByte(rxfifo_t *pFifo) {
	assert_param(pFifo);
  uint8_t tail = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
	if (tail == pFifo->head) {
			return -1;
	} else {
			return pFifo->pbuf[pFifo->head];
	}
}
 
/*
Считывание байта
-1 - empty
*/
int16_t rxFifoReadByte (rxfifo_t *pFifo) {
	assert_param(pFifo);
  uint8_t tail = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
	if (tail == pFifo->head) {
			return -1;
	} else {
		  uint8_t byte = pFifo->pbuf[pFifo->head++];
			if (pFifo->head >= pFifo->size) pFifo->head = 0;
			return byte;
	}
}
/*
Считывание указанного количества принятых байтов (или максимально доступного)
*/
uint8_t rxFifoReadBytes (rxfifo_t *pFifo, uint8_t *pBuffer, uint8_t bufLen) {
	assert_param(pFifo);
	assert_param(pBuffer);
  uint8_t tail = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
  uint8_t i;
 
	for (i=0;i<bufLen;i++) {
		if (tail == pFifo->head) {
				return i;
		}
		*pBuffer++ = pFifo->pbuf[pFifo->head++];
		if (pFifo->head >= pFifo->size) pFifo->head = 0;
	}
	return i;
}
 
/*
Считывание "строки" до символа разделителя. Если символ не найден, то return 0.
*/
uint32_t rxFifoReadBytesUntil (rxfifo_t *pFifo, uint8_t* pBuffer, uint8_t bufLen, uint8_t delimiter) {
	assert_param(pFifo);
	assert_param(pBuffer);
  uint8_t tail = pFifo->size - pFifo->uartHandle->hdmarx->Instance->NDTR;
  uint8_t savedHead = pFifo->head;
  uint8_t i;
  uint8_t byte;
 
	for (i=0;i<bufLen;i++) {
		if (tail == pFifo->head) {              // End of internal buffer found before delimiter
				pFifo->head  = savedHead;          // Restore internal buffer
				return 0;
		}
		byte = pFifo->pbuf[pFifo->head++];
		if (pFifo->head >= pFifo->size) pFifo->head = 0;
		*pBuffer++ = byte;
		if (byte == delimiter) {
				return i+1;
		}
	}
	return i;
}
////////////////////////// Transmit ///////////////////////////////
/* Запуск передачи по UART через DMA. Предварительно UART & DMA должны быть связаны в функции настройки */
int txFifoStart(txfifo_t *pFifo)
{
	assert_param(pFifo);
	HAL_UART_StateTypeDef state;
	// надо проверить, что передача еще не запущена и тогда запускать...
	state = HAL_UART_GetState(pFifo->uartHandle);
	if(state == HAL_UART_STATE_BUSY_TX_RX || state == HAL_UART_STATE_BUSY_TX){
		return -1;
	}
	if(pFifo->tail == 0){
		return -1;		
	}
#ifdef USE_CALLBACK
	if(pFifo->pBeforeTransmitCallback) pFifo->pBeforeTransmitCallback();
#endif
	if(HAL_UART_Transmit_DMA(pFifo->uartHandle, (uint8_t*)pFifo->pbuf, pFifo->tail) != HAL_OK){
		return -1;
	}
	// сменим буфер для накопления данных
	if(pFifo->pbuf == pFifo->pbuf1){
		pFifo->pbuf = pFifo->pbuf2;
	}else{
		pFifo->pbuf = pFifo->pbuf1;
	}
	pFifo->tail=0;
	return 0;
}

void txFifoClear (txfifo_t *pFifo) {
	assert_param(pFifo);
	pFifo->tail = 0;
}

int txFifoPutChar(txfifo_t *pFifo, uint8_t c)
{
	assert_param(pFifo);
	if(pFifo->tail >= pFifo->size){
		return -1;
	}
	pFifo->pbuf[pFifo->tail++] = c;
	// запустим передачу, если возможно
#ifdef START_SEND_IMMEDIATE
	txFifoStart(pFifo);
#endif
	return 0;
}

int txFifoPutStr(txfifo_t *pFifo, uint8_t* pc, uint8_t cnt)
{
	assert_param(pFifo);
	assert_param(pc);
	if(cnt > (pFifo->size - pFifo->tail)){
		return -1;
	}
	for(int i=0; i<cnt; i++){
		pFifo->pbuf[pFifo->tail++] = pc[i];
	}
	// запустим передачу, если возможно
#ifdef START_SEND_IMMEDIATE
	txFifoStart(pFifo);
#endif
	return 0;
}

/*
Количество принятых и не считанных байтов
*/
uint8_t txFifoFreeSpace (txfifo_t *pFifo) {
	assert_param(pFifo);
  return (pFifo->size) - (pFifo->tail);
}

/*
сигнал завершения выдачи из FIFO.
Нужно определить необходимость продолжения выдачи из второй половины буфера, либо остановиться
*/
void txFifoCpltCallback (txfifo_t *pFifo)
{
	assert_param(pFifo);
	// если нечего больше передавать, то дадим сигнал о завершении выдачи
	if(pFifo->tail == 0){
#ifdef USE_CALLBACK
		if(pFifo->pEndTransmitCallback) pFifo->pEndTransmitCallback();
#endif
	}
	else{
		txFifoStart(pFifo);
	}
}


