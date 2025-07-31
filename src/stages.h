#ifndef __STAGES_H__
#define __STAGES_H__

	/**
	 * @brief A struct for each stage used by the software.
	 */
	typedef struct stage_info
	{
		char *name;
		s32 id;
		void (*init)();
		void (*update)();
		void (*render)();
		void (*destroy)();
	} StageInfo;

	/**
	 * @brief The list of stages made available for the software.
	 */
	extern StageInfo stages[6];

	/**
	 * @brief Changes the target_stage value, effectively creating a
	 * request to the engine loop to initiate change_stage().
	 *
	 * @param name    The name of the target stage.
	 */
	void request_stage_change(const char *name);

	/**
	 * @brief A function for exclusive use by the main thread, to be
	 * called whenever a video retrace signal is sent to the scheduler.
	 *
	 * This function tells the software to trigger the current stage's
	 * update function and then render a new frame.
	 */
	// void update_current_stage();

#endif