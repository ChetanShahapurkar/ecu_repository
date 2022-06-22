

// //Maximum stack size allowed for system task
// #define STACK_SYSTEM_TASK_SIZE  4096 

// //Maximum stack size allowed for input task
// #define STACK_INPUT_TASK_SIZE  4096 

// //Maximum stack size allowed for output task
// #define STACK_OUTPUT_TASK_SIZE  4096 

// //Maximum stack size allowed for input output task
// #define STACK_INPUTOUTPUT_TASK_SIZE  4096 

// //A constant pointer to a function with no return value or parameters.
// typedef void (* const TASK_TYPE)(void);

// //A constant pointer to a list of TASK_TYPESs.
// typedef void (* const * const TASK_LIST_TYPE)(void);

// //A NULL pointer to use as a terminator for function lists.
// #define NULL_TASK ((TASK_TYPE)0)


// //=================================================================================================================================
// //------------------------------------------------ SYSTEM INIT TASKS --------------------------------------------------------------
// //=================================================================================================================================
// /** @brief      List of the system tasks executed before entering the main loop.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE SystemInitialization_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
		
// 		NULL_TASK
// };


// //=================================================================================================================================
// //------------------------------------------------ SYSTEM INPUT INIT TASKS --------------------------------------------------------
// //=================================================================================================================================

// /** @brief      List of the input tasks executed before entering the main loop.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE InputsInitialization_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
		
// 		NULL_TASK
// };

// //=================================================================================================================================
// //------------------------------------------------ SYSTEM OUTPUT INIT TASKS -------------------------------------------------------
// //=================================================================================================================================

// /** @brief      List of the input tasks executed before entering the main loop.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE OutputsInitialization_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
// 		// stepperMotor__Init,
// 		NULL_TASK
// };

// //=================================================================================================================================
// //------------------------------------------------ SYSTEM INPUT OUTPUT INIT TASKS -------------------------------------------------
// //=================================================================================================================================

// /** @brief      List of the input tasks executed before entering the main loop.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE InputOutputsInitialization_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
		
// 		NULL_TASK
// };


// //=================================================================================================================================
// //------------------------------------------------ SYSTEM INPUT HANDLER TASKS -------------------------------------------------
// //=================================================================================================================================

// /** @brief      List of the input tasks executed continuously.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE InputHandler_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
		
// 		NULL_TASK
// };

// //=================================================================================================================================
// //------------------------------------------------ SYSTEM OUTPUT HANDLER TASKS -------------------------------------------------
// //=================================================================================================================================

// /** @brief      List of the output tasks executed continuously.
//  *  @details    Add pointers to routines that should be called during initialization.  These
//  *              routines are intended to initialize or begin the initialization process for all the
//  *              modules in the application.
//  */
// TASK_TYPE OutputHandler_Tasks[] =
// {
// 		//-------------------------------------------------------------------------
// 		// Mandatory NULL_TASK must be at the end of the list
// 		// stepperMotor__Handler,
// 		NULL_TASK
// };