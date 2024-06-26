//
// SPDX-License-Identifier: Apache-2.0
// [LANL Copyright Statement]
// Copyright (C) 2023 - 2024 by the ryujin authors
// Copyright (C) 2023 - 2024 by Triad National Security, LLC
//

#pragma once

#include "hyperbolic_system.h"

#include <simd.h>

#include <deal.II/base/point.h>
#include <deal.II/base/tensor.h>

namespace ryujin
{
  namespace ShallowWater
  {
    template <typename ScalarNumber = double>
    class RiemannSolverParameters : public dealii::ParameterAcceptor
    {
    public:
      RiemannSolverParameters(const std::string &subsection = "/RiemannSolver")
          : ParameterAcceptor(subsection)
      {
      }
    };


    /**
     * A fast approximative solver for the associated 1D Riemann problem.
     * The solver has to ensure that the estimate
     * \f$\lambda_{\text{max}}\f$ that is returned for the maximal
     * wavespeed is a strict upper bound.
     *
     * @ingroup ShallowWaterEquations
     */
    template <int dim, typename Number = double>
    class RiemannSolver
    {
    public:
      /**
       * @name Typedefs and constexpr constants
       */
      //@{

      using View = HyperbolicSystemView<dim, Number>;

      using ScalarNumber = typename View::ScalarNumber;

      static constexpr auto problem_dimension = View::problem_dimension;

      using state_type = typename View::state_type;

      /**
       * Number of components in a primitive state, we store \f$[\rho, v,
       * p, a]\f$, thus, 4.
       */
      static constexpr unsigned int riemann_data_size = 3;

      /**
       * The array type to store the expanded primitive state for the
       * Riemann solver \f$[\rho, v, p, a]\f$
       */
      using primitive_type = typename std::array<Number, riemann_data_size>;

      using precomputed_type = typename View::precomputed_type;

      using PrecomputedVector = typename View::PrecomputedVector;

      using Parameters = RiemannSolverParameters<ScalarNumber>;

      //@}
      /**
       * @name Compute wavespeed estimates
       */
      //@{

      /**
       * Constructor taking a HyperbolicSystem instance as argument
       */
      RiemannSolver(const HyperbolicSystem &hyperbolic_system,
                    const Parameters &parameters,
                    const PrecomputedVector &precomputed_values)
          : hyperbolic_system(hyperbolic_system)
          , parameters(parameters)
          , precomputed_values(precomputed_values)
      {
      }

      /**
       * For two given 1D primitive states riemann_data_i and riemann_data_j,
       * compute an estimation of an upper bound for the maximum wavespeed
       * lambda.
       */
      Number compute(const primitive_type &riemann_data_i,
                     const primitive_type &riemann_data_j) const;

      /**
       * For two given states U_i a U_j and a (normalized) "direction" n_ij
       * compute an estimation of an upper bound for lambda.
       */
      Number compute(const state_type &U_i,
                     const state_type &U_j,
                     const unsigned int i,
                     const unsigned int *js,
                     const dealii::Tensor<1, dim, Number> &n_ij) const;

    protected:
      //@}
      /**
       * @name Internal functions used in the Riemann solver
       */
      //@{

      Number f(const primitive_type &primitive_state,
               const Number &h_star) const;

      Number phi(const primitive_type &riemann_data_i,
                 const primitive_type &riemann_data_j,
                 const Number &h) const;

      Number lambda1_minus(const primitive_type &riemann_data,
                           const Number h_star) const;

      Number lambda3_plus(const primitive_type &riemann_data,
                          const Number h_star) const;

      Number compute_lambda(const primitive_type &riemann_data_i,
                            const primitive_type &riemann_data_j,
                            const Number h_star) const;

    public:
      Number compute_h_star(const primitive_type &riemann_data_i,
                            const primitive_type &riemann_data_j) const;

    protected:
      primitive_type
      riemann_data_from_state(const state_type &U,
                              const dealii::Tensor<1, dim, Number> &n_ij) const;

    private:
      const HyperbolicSystem &hyperbolic_system;
      const Parameters &parameters;
      const PrecomputedVector &precomputed_values;
      //@}
    };
  } // namespace ShallowWater
} // namespace ryujin
