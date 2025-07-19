#ifndef __STAGES_H__
#define __STAGES_H__

	/**
	 * @brief A struct for each stage used by the software.
	 */
	typedef struct stage_info
	{
		u32 id;
		void (*init)();
		void (*update)();
		void (*render)();
		void (*destroy)();
	} StageInfo;

	/**
	 * @brief The list of stages made available for the software.
	 */
	extern StageInfo stages[];

	/**
	 * @brief A struct for exclusive use by the main thread.
	 */
	extern u32 current_stage;
	extern u32 target_stage;
	extern int current_stage_index;

	/**
	 * @brief A function for exclusive use by the main thread.
	 * 
	 * This sets current_stage to the value specified in target and calls
	 * the init function of the target stage.
	 *
	 * @param target_ID    The ID of the target stage.
	 */
	extern void change_stage(u32 id);

	/**
	 * @brief Checks if target_stage is not equal to current_stage.
	 */
	extern bool change_stage_needed();

	/**
	 * @brief A function for exclusive use by the main thread, to be
	 * called whenever a video retrace signal is sent to the scheduler.
	 *
	 * This function tells the software to trigger the current stage's
	 * update function and then render a new frame.
	 */
	// extern void update_current_stage();

#endif