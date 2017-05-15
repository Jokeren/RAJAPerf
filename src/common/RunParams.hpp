/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RunParams class.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-xxxxxx
//
// All rights reserved.
//
// This file is part of the RAJA Performance Suite.
//
// For additional details, please read the file LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJAPerf_RunParams_HPP
#define RAJAPerf_RunParams_HPP

#include <string>
#include <vector>
#include <iosfwd>

namespace rajaperf
{

/*!
 *******************************************************************************
 *
 * \brief Simple class to hold suite execution parameters.
 *
 *******************************************************************************
 */
class RunParams {

public:
  RunParams( int argc, char** argv );
  ~RunParams( );

  /*!
   * \brief Enumeration indicating state of input options requested
   */
  enum InputOpt {
    InfoRequest,  /*!< option requesting information */
    DryRun,       /*!< report summary of how suite will run w/o running */
    GoodToRun,    /*!< input defines a valid run, suite will run */
    BadInput,     /*!< erroneous input given */ 
    Undefined     /*!< input not defined (yet) */
  };

//@{
//! @name Methods to get/set input state

  InputOpt getInputState() const { return input_state; } 

  /*!
   * \brief Set whether run parameters (from input) are valid.
   */
  void setInputState(InputOpt is) { input_state = is; }

//@}


//@{
//! @name Getters/setters for processing input

  int getNumPasses() const { return npasses; }

  double getSampleFraction() const { return sample_fraction; }

  double getSizeFraction() const { return size_fraction; }

  const std::string& getReferenceVariant() const { return reference_variant; }

  const std::vector<std::string>& getKernelInput() const 
                                  { return kernel_input; }
  void setInvalidKernelInput( std::vector<std::string>& svec )
                              { invalid_kernel_input = svec; }
  const std::vector<std::string>& getInvalidKernelInput() const
                                  { return invalid_kernel_input; }

  const std::vector<std::string>& getVariantInput() const 
                                  { return variant_input; }
  void setInvalidVariantInput( std::vector<std::string>& svec )
                               { invalid_variant_input = svec; }
  const std::vector<std::string>& getInvalidVariantInput() const
                                  { return invalid_variant_input; }

  const std::string& getOutputDirName() const { return outdir; }
  const std::string& getOutputFilePrefix() const { return outfile_prefix; }

//@}

  /*!
   * \brief Print all run params data to given output stream.
   */
  void print(std::ostream& str) const;


private:
  RunParams() = delete;

//@{
//! @name Routines used in command line parsing
  void parseCommandLineOptions(int argc, char** argv);
  void printHelpMessage(std::ostream& str) const;
  void printFullKernelNames(std::ostream& str) const;
  void printKernelNames(std::ostream& str) const;
  void printVariantNames(std::ostream& str) const;
  void printGroupNames(std::ostream& str) const;
//@}

  InputOpt input_state;            /*!< state of command line input */

  int npasses;                     /*!< Number of passes through suite  */
  double sample_fraction;          /*!< Frac of default kernel samples to run */
  double size_fraction;            /*!< Frac of default kernel iteration space
                                        to run */

  std::string reference_variant;   /*!< Name of reference variant for speedup
                                        calculations */ 

  //
  // Arrays to hold input strings for valid/invalid input. Helpful for  
  // debugging command line args.
  //
  std::vector<std::string> kernel_input;
  std::vector<std::string> invalid_kernel_input;
  std::vector<std::string> variant_input;
  std::vector<std::string> invalid_variant_input;

  std::string outdir;          /*!< Output directory name. */
  std::string outfile_prefix;  /*!< Prefix for output data file names. */

};


}  // closing brace for rajaperf namespace

#endif  // closing endif for header file include guard