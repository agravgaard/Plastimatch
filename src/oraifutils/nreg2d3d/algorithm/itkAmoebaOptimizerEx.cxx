//
//
// NOTE:
// This class is a simple extension of the original ITK class itk::AmoebaOptimizer
// in order to support stopping the internal amoeba-VNL-optimizer. This class
// only works together with the extended version of the amoeba-VNL-optimizer.
// @author phil <philipp.steininger (at) pmu.ac.at>
// @version 1.0
//
//
/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkAmoebaOptimizer.cxx,v $
  Language:  C++
  Date:      $Date: 2009-09-12 20:00:29 $
  Version:   $Revision: 1.33 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkAmoebaOptimizerEx.h"

namespace itk
{

/**
 * Constructor
 */
AmoebaOptimizerEx
::AmoebaOptimizerEx()
  : m_InitialSimplexDelta(1)  // initial size
{
  m_OptimizerInitialized           = false;
  m_VnlOptimizer                   = 0;
  m_MaximumNumberOfIterations      = 500;
  m_ParametersConvergenceTolerance = 1e-8;
  m_FunctionConvergenceTolerance   = 1e-4;
  m_AutomaticInitialSimplex        = true;
  m_InitialSimplexDelta.Fill(NumericTraits<ParametersType::ValueType>::One);
}


/**
 * Destructor
 */
AmoebaOptimizerEx
::~AmoebaOptimizerEx()
{
  delete m_VnlOptimizer;
}

const std::string
AmoebaOptimizerEx
::GetStopConditionDescription() const
{
  return m_StopConditionDescription.str();
}

/**
 * PrintSelf
 */
void
AmoebaOptimizerEx
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "MaximumNumberOfIterations: " 
     << m_MaximumNumberOfIterations << std::endl;
  os << indent << "ParametersConvergenceTolerance: "
     << m_ParametersConvergenceTolerance << std::endl;
  os << indent << "FunctionConvergenceTolerance: "
     << m_FunctionConvergenceTolerance << std::endl;
  os << indent << "AutomaticInitialSimplex: "
     << (m_AutomaticInitialSimplex ? "On" : "Off") << std::endl;
  os << indent << "InitialSimplexDelta: "
     << m_InitialSimplexDelta << std::endl;
}
  
/** Return Current Value */
AmoebaOptimizerEx::MeasureType
AmoebaOptimizerEx
::GetValue() const
{
  ParametersType parameters = this->GetCurrentPosition();
  if(m_ScalesInitialized)
    {
    const ScalesType scales = this->GetScales();
    for(unsigned int i=0;i<parameters.size();i++)
      {
      parameters[i] *= scales[i]; 
      }
    }
  return this->GetNonConstCostFunctionAdaptor()->f( parameters );
}

/**
 * Set the maximum number of iterations
 */
void
AmoebaOptimizerEx
::SetMaximumNumberOfIterations( unsigned int n )
{
  if ( n == m_MaximumNumberOfIterations )
    {
    return;
    }

  m_MaximumNumberOfIterations = n;
  if ( m_OptimizerInitialized )
    {
    m_VnlOptimizer->set_max_iterations( static_cast<int>( n ) );
    }

  this->Modified();
}

/**
 * Set the parameters convergence tolerance
 */
void
AmoebaOptimizerEx
::SetParametersConvergenceTolerance( double tol )
{
  if ( tol == m_ParametersConvergenceTolerance )
    {
    return;
    }

  m_ParametersConvergenceTolerance = tol;
  if ( m_OptimizerInitialized )
    {
    m_VnlOptimizer->set_x_tolerance( tol );
    }

  this->Modified();
}


/**
 * Set the function convergence tolerance
 */
void
AmoebaOptimizerEx
::SetFunctionConvergenceTolerance( double tol )
{
  if ( tol == m_FunctionConvergenceTolerance )
    {
    return;
    }

  m_FunctionConvergenceTolerance = tol;
  if ( m_OptimizerInitialized )
    {
    m_VnlOptimizer->set_f_tolerance( tol );
    }

  this->Modified();
}

/**
 * Connect a Cost Function
 */
void
AmoebaOptimizerEx
::SetCostFunction( SingleValuedCostFunction * costFunction )
{
  const unsigned int numberOfParameters = 
    costFunction->GetNumberOfParameters();

  CostFunctionAdaptorType * adaptor = 
    new CostFunctionAdaptorType( numberOfParameters );
       
  SingleValuedNonLinearOptimizer::SetCostFunction( costFunction );
  adaptor->SetCostFunction( costFunction );

  if( m_OptimizerInitialized )
    { 
    delete m_VnlOptimizer;
    }
    
  this->SetCostFunctionAdaptor( adaptor );

  m_VnlOptimizer = new vnl_amoeba_ex( *adaptor );

  // set up optimizer parameters
  m_VnlOptimizer->set_max_iterations( static_cast<int>( m_MaximumNumberOfIterations ) );
  m_VnlOptimizer->set_x_tolerance( m_ParametersConvergenceTolerance );
  m_VnlOptimizer->set_f_tolerance( m_FunctionConvergenceTolerance );

  m_OptimizerInitialized = true;

}

/**
 * Start the optimization
 */
void
AmoebaOptimizerEx
::StartOptimization( void )
{
    
  this->InvokeEvent( StartEvent() );
  m_StopConditionDescription.str("");
  m_StopConditionDescription << this->GetNameOfClass() << ": Running";

  if( this->GetMaximize() )
    {
    this->GetNonConstCostFunctionAdaptor()->NegateCostFunctionOn();
    }

  ParametersType initialPosition = this->GetInitialPosition();
  this->SetCurrentPosition( initialPosition );

  ParametersType parameters( initialPosition );

  // If the user provides the scales then we set otherwise we don't
  // for computation speed.
  // We also scale the initial parameters up if scales are defined.
  // This compensates for later scaling them down in the cost function adaptor
  // and at the end of this function.  
  if(m_ScalesInitialized)
    {
    ScalesType scales = this->GetScales();
    this->GetNonConstCostFunctionAdaptor()->SetScales(scales);
    for(unsigned int i=0;i<parameters.size();i++)
      {
      parameters[i] *= scales[i]; 
      }
    }
  
  
  // vnl optimizers return the solution by reference 
  // in the variable provided as initial position
  if (m_AutomaticInitialSimplex)
    {
    m_VnlOptimizer->minimize( parameters );
    }
  else
    {
    InternalParametersType delta( m_InitialSimplexDelta );
    // m_VnlOptimizer->verbose = 1;
    m_VnlOptimizer->minimize( parameters, delta );
    }
  
  // we scale the parameters down if scales are defined
  if(m_ScalesInitialized)
    {
    ScalesType scales = this->GetScales();
    for(unsigned int i=0;i<parameters.size();i++)
      {
      parameters[i] /= scales[i]; 
      }
    }

  this->SetCurrentPosition( parameters );
    
  if (m_StopConditionDescription.str().length() > 0)
  {
    m_StopConditionDescription.str("");
    m_StopConditionDescription << this->GetNameOfClass() << ": ";
    if (static_cast<unsigned int>(m_VnlOptimizer->get_num_evaluations())
        < m_MaximumNumberOfIterations)
      {
      m_StopConditionDescription << "Both parameters convergence tolerance ("
                                 << m_ParametersConvergenceTolerance
                                 << ") and function convergence tolerance ("
                                 << m_FunctionConvergenceTolerance
                                 << ") have been met in "
                                 << m_VnlOptimizer->get_num_evaluations()
                                 << " iterations.";
      }
    else
      {
      m_StopConditionDescription << "Maximum number of iterations exceeded."
                                 << " Number of iterations is "
                                 << m_MaximumNumberOfIterations;

      }
  }
  else
  {
    m_StopConditionDescription << "Manual stop";
  }
  this->InvokeEvent( EndEvent() );
}

/**
 * Get the Optimizer
 */
vnl_amoeba_ex *
AmoebaOptimizerEx
::GetOptimizer()
{
  return m_VnlOptimizer;
}

void AmoebaOptimizerEx::StopOptimization()
{
  if (GetOptimizer() && GetOptimizer()->fitter)
  {
    vnl_amoeba_ex *amoeba = dynamic_cast<vnl_amoeba_ex *>(GetOptimizer()->fitter);
    if (amoeba)
      amoeba->stop = true; // stop optimization!!!
    m_StopConditionDescription.str(""); // mark stop!
  }
}

} // end namespace itk
