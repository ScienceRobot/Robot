//win_Tasks.h
#ifndef _WIN_TASKS_H
#define _WIN_TASKS_H

//#include "Robot.h"  //for EthAccelTouch_PCB 
#include "win_Load_Script.h" //for RobotScript


//structure to store variables for the thread that runs tasks
//for example a keypress detected in the keypress call back function will set a flag and store the key in this structure, so that the task thread can retrieve the key and determine what to do with the current running task (script)- end the loop or stop the script, etc.
#define ROBOT_TASK_ENGINE_KEYPRESS				0x00000001 //key was pressed
#define ROBOT_TASK_ENGINE_ABORTING_TASK		0x00000002 //AbortTask was called- used to make sure AbortTask does not process more than once per task
typedef struct RobotTaskEngine
{
uint32_t flags;  //see above flags
KeySym key; //key that was pressed
} RobotTaskEngine;


typedef struct RobotInstructionList
{
RobotInstruction *Inst; //pointer to a Robot Instruction
struct RobotInstructionList *next; //pointer to the next Robot Instruction
} RobotInstructionList;


//structure for linked list of Robot tasks
//alternative names:
//RStatus contains iTask which points to this list of active Robot tasks (which currently include running scripts, and balancing)
#define ROBOT_TASK_KIND_SCRIPT 0x1
#define ROBOT_TASK_KIND_BALANCE 0x2

#define ROBOT_TASK_RUNNING											0x00000001	//a robot task (script) is currently running
#define ROBOT_TASK_WAIT_STAGE										0x00000002  //new instructions in a different stage are waiting for the instruction(s) that have a WAIT_STAGE flag to end

#define ROBOT_TASK_WAIT_STAGE_DONE							0x00000004 //indicates that a instruction WAIT_STAGE is done, so to move on to the next instruction
#define ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST	0x00000008 //indicates that the current stage of instructions contains at least 1 instruction that ends when the target range is reached (has no HOLD_ANGLE flag) and so all HoldAngleUntil instructions will end when all instructions without the HOLD_ANGLE flag of this stage end. Perhaps change to WAIT_STAGE_HAS_REQUIRED_INST 
//alt: ROBOT_TASK_WAIT_STAGE_HAS_END_ON_TARGET_INST, ROBOT_TASK_WAIT_STAGE_HAS_END_WHEN_TARGET_ANGLE_REACHED_INST
//#define ROBOT_TASK_WAIT_ALL		0x00000004	//all new instructions are waiting for the current instruction to end
#define ROBOT_TASK_LOOPING											0x00000010 //indicates that a group of instructions is being looped 
#define ROBOT_TASK_KEY_ENDING_LOOP							0x00000020 //a KeyPress is ending a loop
#define ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE		0x00000040 //indicates that a AnalogSensor instruction had its condition met, and has therefore ended a stage of instructions
#define ROBOT_TASK_ABORT_TASK										0x00000080 //abort the task	
typedef struct RobotTask
{
uint32_t flags;
uint32_t kind; //kind of task (script, or balance)
uint32_t StartTime; //time this RobotTask was started (the script time is added to determine when to send the Robot instruction)
uint32_t TaskTime;  //the time a task (script) has been running in ms
uint32_t StageNumber; //(Original Instruction StartTime of the current stage of instructions- used to determine if new insts should be executed or wait- only matching those insts with the same original start (stage) time. alt: StageInstOrigStartTime
//uint32_t StageStartTime; //Actual time stage started (in ms)
//uint32_t StageDuration;
int32_t LoopCount; //Current Loop Count (starts at 1)
int32_t LoopEndCount; //Last Loop Count (when to stop looping) -1=never
uint32_t LoopStartTime;  //CurrentTime when the last StartLoop inst was started - used to push back all later inst start times when a loop is in a script
unsigned int KeyCode[2]; //Loop KeyCodes  0=KeyCode1 (on keypress end loop after last inst) 1=KeyCode2 (on keypress end loop instantly)
RobotScript *iScript; //pointer to script (only for tasks that are scripts)
RobotInstruction *CurrentRobotInst; //pointer to the next RobotInstruction to be sent
RobotInstructionList *iCond; //list of pointers to Robot Instructions with conditions still being checked
RobotInstruction *StartLoopInst; //pointer to the first inst in a loop (only used during looping)
struct RobotTask *next;
} RobotTask;


int winTasks_AddFTWindow(void);
int winTasks_AddFTControls(FTWindow *twin);
void winTasks_key(FTWindow *twin,KeySym key);
int winTasks_OnOpen(FTWindow *twin);
void btnwinTasksClose_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnStand_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnLeanLeft_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnLeanRight_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnLeanForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnLeanBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnStartAllAccelAndTouch_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnStartAllAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int StartAllAccelsAndAnalogSensors(int IncludeAnalog);
int StopAllAccelsAndAnalogSensors(int IncludeAnalog);
int btnLeftStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRightStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnLoadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnReloadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnUnloadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRunScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnBalance_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
#if Linux
int Thread_BalanceRobot(void);
#endif
#if WIN32
DWORD WINAPI Thread_BalanceRobot(LPVOID lpParam);
#endif
int StartBalancing(void);
int StopBalancing(void);
int StepLeft(void);
int AddRobotTask(RobotTask *ltask);
int RemoveRobotTask(RobotTask *ltask);
int FreeRobotTask(RobotTask *ltask);
int FreeAllRobotTasks(void);
//change to AddRobotInstructionToList()? or just AddInstructionToList()
int AddCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst); 
int DeleteCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst);
int DeleteAllCondInstructions(RobotInstructionList **linstlist);
int CreateRobotTasksThread(void);
int StopRobotTasksThread(void);
#if USE_RT_PREEMPT
int CreateTurnMotorThread(void);
int StopTurnMotorThread(void);
#endif
int AbortTask(RobotTask **pltask);
void IncreaseThrust(EthMotorsInstruction *lemi,int AngNeedsToInc,int IncAngleDir,int UseAngle);
void DecreaseThrust(EthMotorsInstruction *lemi,int AngNeedsToInc,int IncAngleDir,int Amount,int UseAngle);
#if Linux
int Thread_RobotTasks(void); 
#endif
#if WIN32
DWORD WINAPI Thread_RobotTasks(LPVOID lpParam);
#endif
void ProcessCondInsts(RobotTask *ltask);

#endif // _WIN_TASKS_H 
