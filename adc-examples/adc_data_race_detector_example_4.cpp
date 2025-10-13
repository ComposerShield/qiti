
/******************************************************************************
 * TSan output (abridged)
 ******************************************************************************/

// ==================
// WARNING: ThreadSanitizer: data race
//   Write of size 4 at 0x000100948000 by thread T2:
//     #0 operator() adc_data_race_detector_example_1.cpp:16
//
//   Previous write of size 4 at 0x000100948000 by thread T1:
//     #0 operator() adc_data_race_detector_example_1.cpp:15
//
//   Location is global 'value' at 0x000100948000
//
// SUMMARY: ThreadSanitizer: data race
//          adc_data_race_detector_example_1.cpp:16
// ==================
