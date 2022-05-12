//
// SPDX-License-Identifier: MIT
// Copyright (C) 2020 - 2022 by the ryujin authors
//

#pragma once

#include <hyperbolic_system.h>

#include <compile_time_options.h>
#include <multicomponent_vector.h>
#include <simd.h>

#include <deal.II/base/vectorization.h>


namespace ryujin
{
  template <int dim, typename Number = double>
  class Indicator
  {
  public:
    /**
     * @copydoc HyperbolicSystem::problem_dimension
     */
    static constexpr unsigned int problem_dimension =
        HyperbolicSystem::problem_dimension<dim>;

    /**
     * @copydoc HyperbolicSystem::state_type
     */
    using state_type = HyperbolicSystem::state_type<dim, Number>;

    /**
     * @copydoc HyperbolicSystem::flux_type
     */
    using flux_type = HyperbolicSystem::flux_type<dim, Number>;

    /**
     * @copydoc HyperbolicSystem::ScalarNumber
     */
    using ScalarNumber = typename get_value_type<Number>::type;

    /**
     * @name Precomputation of indicator quantities
     */
    //@{

    /**
     * The number of precomputed values.
     */
    static constexpr unsigned int n_precomputed_values = 1;

    /**
     * Array type used for precomputed values.
     */
    using precomputed_type = std::array<Number, n_precomputed_values>;

    /**
     * Precomputed values for a given state.
     */
    static precomputed_type
    precompute_values(const HyperbolicSystem &hyperbolic_system,
                      const state_type &U);

    //@}
    /**
     * @name Stencil-based computation of indicators
     *
     * Intended usage:
     * ```
     * Indicator<dim, Number> indicator;
     * for (unsigned int i = n_internal; i < n_owned; ++i) {
     *   // ...
     *   indicator.reset(i, U_i);
     *   for (unsigned int col_idx = 1; col_idx < row_length; ++col_idx) {
     *     // ...
     *     indicator.add(js, U_j, c_ij);
     *   }
     *   indicator.alpha(hd_i);
     * }
     * ```
     */
    //@{

    /**
     * Constructor taking a HyperbolicSystem instance as argument
     */
    Indicator(const HyperbolicSystem &hyperbolic_system,
              const MultiComponentVector<ScalarNumber, n_precomputed_values>
                  &precomputed_values)
        : hyperbolic_system(hyperbolic_system)
        , precomputed_values(precomputed_values)
    {
    }

    /**
     * Reset temporary storage and initialize for a new row corresponding
     * to state vector U_i.
     */
    void reset(const unsigned int i, const state_type &U_i);

    /**
     * When looping over the sparsity row, add the contribution associated
     * with the neighboring state U_j.
     */
    void add(const unsigned int *js,
             const state_type &U_j,
             const dealii::Tensor<1, dim, Number> &c_ij);

    /**
     * Return the computed alpha_i value.
     */
    Number alpha(const Number h_i);

    //@}

  private:
    /**
     * @name
     */
    //@{

    const HyperbolicSystem &hyperbolic_system;

    const MultiComponentVector<ScalarNumber, n_precomputed_values>
        &precomputed_values;

    Number h_i = 0.;
    Number eta_i = 0.;
    flux_type f_i;
    state_type d_eta_i;
    Number pressure_i = 0.;

    Number left = 0.;
    state_type right;

    //@}
  };


  template <int dim, typename Number>
  DEAL_II_ALWAYS_INLINE inline typename Indicator<dim, Number>::precomputed_type
  Indicator<dim, Number>::precompute_values(
      const HyperbolicSystem &hyperbolic_system, const state_type &U_i)
  {
    precomputed_type result;
    result[0] = hyperbolic_system.mathematical_entropy(U_i);
    return result;
  }


  template <int dim, typename Number>
  DEAL_II_ALWAYS_INLINE inline void
  Indicator<dim, Number>::reset(const unsigned int i, const state_type &U_i)
  {
    /* entropy viscosity commutator: */

    const auto &[mathematical_entropy] =
        precomputed_values.template get_tensor<Number, precomputed_type>(i);

    eta_i = mathematical_entropy;

    d_eta_i = hyperbolic_system.mathematical_entropy_derivative(U_i);
    f_i = hyperbolic_system.f(U_i);
    pressure_i = hyperbolic_system.pressure(U_i);

    left = 0.;
    right = 0.;
  }


  template <int dim, typename Number>
  DEAL_II_ALWAYS_INLINE inline void
  Indicator<dim, Number>::add(const unsigned int *js,
                              const state_type &U_j,
                              const dealii::Tensor<1, dim, Number> &c_ij)
  {
    /* entropy viscosity commutator: */

    const auto &[eta_j] =
        precomputed_values.template get_tensor<Number, precomputed_type>(js);

    const auto velocity_j = hyperbolic_system.momentum(U_j) *
                            hyperbolic_system.inverse_water_depth(U_j);
    const auto f_j = hyperbolic_system.f(U_j);
    const auto pressure_j = hyperbolic_system.pressure(U_j);

    left += (eta_j + pressure_j) * (velocity_j * c_ij);

    for (unsigned int k = 0; k < problem_dimension; ++k)
      right[k] += (f_j[k] - f_i[k]) * c_ij;
  }


  template <int dim, typename Number>
  DEAL_II_ALWAYS_INLINE inline Number
  Indicator<dim, Number>::alpha(const Number hd_i)
  {
    using ScalarNumber = typename get_value_type<Number>::type;

    const auto numerator = std::abs(left - d_eta_i * right);
    const auto denominator = std::abs(left) + std::abs(d_eta_i * right);

    /* FIXME: this can be refactoring into a runtime parameter... */
    const ScalarNumber evc_alpha_0_ = ScalarNumber(1.);

    constexpr ScalarNumber eps = std::numeric_limits<ScalarNumber>::epsilon();
    const auto regularization =
        hd_i * (std::abs(eta_i) + std::abs(pressure_i) + eps);

    const auto quotient =
        std::abs(numerator + regularization) / (denominator + regularization);
    return std::min(Number(1.), evc_alpha_0_ * quotient);

    return quotient;
  }

} // namespace ryujin