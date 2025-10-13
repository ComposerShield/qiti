/******************************************************************************
 * "Instrument Your Code So it Can Sing"
 ******************************************************************************/

#include <array>

/** Three Instrumentation Tools:
  1. Heap Allocation Tracker
  2. Function Profiler
  3. Data Race Detector
 */
std::array<Tool, 3> toolsWeWillBuild;

void adc_talk_impl()
{
    what_is_instrumentation();
    
    for (auto& tool : toolsWeWillBuild)
    {
        tool->build(); // concisely!
        tool->review();
        tool->show_qiti_example();
    }
}
