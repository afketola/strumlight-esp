#ifndef BT_SERVICE_H
#define BT_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes Classic Bluetooth (BR/EDR only) with SPP.
 */
void initBT(void);

/**
 * @brief Restarts the Bluetooth stack.
 */
void restartBT(void);

#ifdef __cplusplus
}
#endif

#endif // BT_SERVICE_H
