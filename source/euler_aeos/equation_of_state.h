//
// SPDX-License-Identifier: MIT
// Copyright (C) 2020 - 2023 by the ryujin authors
//

#pragma once

#include <compile_time_options.h>

#include "convenience_macros.h"

#include <deal.II/base/array_view.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/parameter_acceptor.h>
#include <deal.II/base/tensor.h>

#include <string>

namespace ryujin
{
  namespace EquationOfStateLibrary
  {
    /**
     * A small abstract base class to group configuration options for an
     * equation of state.
     *
     * @ingroup EulerEquations
     */
    class EquationOfState : public dealii::ParameterAcceptor
    {
    public:
      /**
       * Constructor taking EOS name @p name and a subsection @p subsection
       * as an argument. The dealii::ParameterAcceptor is initialized with
       * the subsubsection `subsection + "/" + name`.
       */
      EquationOfState(const std::string &name, const std::string &subsection)
          : ParameterAcceptor(subsection + "/" + name)
          , name_(name)
      {
        /*
         * If necessary derived EOS can override the interpolation
         * co-volume b that is used in the approximate Riemann solver.
         */
        interpolation_b_ = 0.;

        /*
         * If necessary derived EOS can override this boolean to indicate
         * that the dealii::ArrayView<double> variants of the pressure()
         * function (etc.) should be preferred.
         */
        prefer_vector_interface_ = false;
      }

      /**
       * Return the pressure given density @p rho and specific internal
       * energy @p e.
       */
      virtual double pressure(double rho, double e) = 0;

      /**
       * Variant of above function operating on a contiguous range of
       * values. The result is stored in the first argument @p p,
       * overriding previous contents.
       *
       * @note The second and third arguments are writable as well. We need
       * to perform some unit transformations for certain tabulated
       * equation of state libraries, such as the sesame database. Rather
       * than creating temporaries we override values in place.
       */
      virtual void pressure(const dealii::ArrayView<double> &p,
                            const dealii::ArrayView<double> &rho,
                            const dealii::ArrayView<double> &e)
      {
        Assert(p.size() == rho.size() && rho.size() == e.size(),
               dealii::ExcMessage("vectors have different size"));

        std::transform(std::begin(rho),
                       std::end(rho),
                       std::begin(e),
                       std::begin(p),
                       [&](double rho, double e) { return pressure(rho, e); });
      }

      /**
       * Return the specific internal energy @p e for a given density @p
       * rho and pressure @p p.
       */
      virtual double specific_internal_energy(double rho, double p) = 0;

      /**
       * Variant of above function operating on a contiguous range of
       * values. The result is stored in the first argument @p p,
       * overriding previous contents.
       *
       * @note The second and third arguments are writable as well. We need
       * to perform some unit transformations for certain tabulated
       * equation of state libraries, such as the sesame database. Rather
       * than creating temporaries we override values in place.
       */
      virtual void
      specific_internal_energy(const dealii::ArrayView<double> &e,
                               const dealii::ArrayView<double> &rho,
                               const dealii::ArrayView<double> &p)
      {
        Assert(p.size() == rho.size() && rho.size() == e.size(),
               dealii::ExcMessage("vectors have different size"));

        std::transform(std::begin(rho),
                       std::end(rho),
                       std::begin(p),
                       std::begin(e),
                       [&](double rho, double p) {
                         return specific_internal_energy(rho, p);
                       });
      }

      /**
       * Return the sound speed @p c for a given density @p rho and
       * specific internal energy  @p e.
       */
      virtual double sound_speed(double rho, double e) = 0;

      /**
       * Variant of above function operating on a contiguous range of
       * values. The result is stored in the first argument @p p,
       * overriding previous contents.
       *
       * @note The second and third arguments are writable as well. We need
       * to perform some unit transformations for certain tabulated
       * equation of state libraries, such as the sesame database. Rather
       * than creating temporaries we override values in place.
       */
      virtual void sound_speed(const dealii::ArrayView<double> &c,
                               const dealii::ArrayView<double> &rho,
                               const dealii::ArrayView<double> &e)
      {
        Assert(c.size() == rho.size() && rho.size() == e.size(),
               dealii::ExcMessage("vectors have different size"));

        std::transform(
            std::begin(rho),
            std::end(rho),
            std::begin(e),
            std::begin(c),
            [&](double rho, double e) { return sound_speed(rho, e); });
      }

      /**
       * Return the interpolation co-volume constant (b).
       */
      ACCESSOR_READ_ONLY(interpolation_b)

      /**
       * Return a boolean indicating whether the dealii::ArrayView<double>
       * variants for the pressure(), specific_internal_energy(), and
       * sound_speed() functions should be preferred.
       *
       * Ordinarily we use the single-valued signatures for pre-computation
       * because this leads to slightly better throughput (due to better
       * memory locality with how we store precomputed values) and less
       * memory consumption. On the other hand, some tabulated equation of
       * state libraries work best with a single call and a large dataset.
       */
      ACCESSOR_READ_ONLY(prefer_vector_interface)

      /**
       * Return the name of the EOS as (const reference) std::string
       */
      ACCESSOR_READ_ONLY(name)

    protected:
      double interpolation_b_;
      bool prefer_vector_interface_;

    private:
      const std::string name_;
    };

  } // namespace EulerAEOS
} /* namespace ryujin */
