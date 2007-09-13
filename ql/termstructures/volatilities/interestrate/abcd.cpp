/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/volatilities/interestrate/abcd.hpp>

namespace QuantLib {

//===========================================================================//
//                                  Abcd                                     //
//===========================================================================//

    Abcd::Abcd(Real a, Real b, Real c, Real d)
    : a_(a), b_(b), c_(c), d_(d) {
        validateAbcdParameters(a, b, c, d);
    }

    Real Abcd::operator()(Time u) const {
        return u<0 ? 0.0 : (a_ + b_*u)*std::exp(-c_*u) + d_;
    }

    Real Abcd::maximumLocation() const {
        if (b_<=0) {
            return 0.0;
        } else {
            if((b_-c_*a_)/(c_*b_)>0) {
                return (b_-c_*a_)/(c_*b_);
            } else
                return 0.0;
        }
    }

    Real Abcd::maximumVolatility() const {
        if (b_<=0) {
            return shortTermVolatility();
        } else {
            if ((b_-c_*a_)/(c_*b_) > 0.0) {
                return b_/c_*std::exp(-1.0 +c_*a_/b_) + d_;
            } else
                return shortTermVolatility();
        }
    }

    Real Abcd::volatility(Time tMin, Time tMax, Time T) const {
        if (tMax==tMin)
            return instantaneousVolatility(tMax, T);
        QL_REQUIRE(tMax>tMin, "tMax must be > tMin");
        return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    Real Abcd::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

    Real Abcd::covariance(Time t, Time T, Time S) const {
        return (*this)(T-t) * (*this)(S-t);
    }

    Real Abcd::covariance(Time t1, Time t2, Time T, Time S)
        const {
        QL_REQUIRE(t1<=t2,
                   "integrations bounds (" << t1 <<
                   "," << t2 << ") are in reverse order");
        Time cutOff = std::min(S,T);
        if (t1>=cutOff) {
            return 0.0;
        } else {
            cutOff = std::min(t2, cutOff);
            return primitive(cutOff, T, S) - primitive(t1, T, S);
        }
    }

    // INSTANTANEOUS
    Real Abcd::instantaneousVolatility(Time u, Time T) const {
        return std::sqrt(instantaneousVariance(u, T));
    }

    Real Abcd::instantaneousVariance(Time u, Time T) const {
        return instantaneousCovariance(u, T, T);
    }
    Real Abcd::instantaneousCovariance(Time u, Time T, Time S) const {
        return (*this)(T-u)*(*this)(S-u);
    }

    // PRIMITIVE
    Real Abcd::primitive(Time t, Time T, Time S) const {
        if (T<t || S<t) return 0.0;

        Real k1=std::exp(c_*t), k2=std::exp(c_*S), k3=std::exp(c_*T);

        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*t) + 2*c_*c_*(S - t)*(T - t)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*t)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*t)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - t))
                               - k1*k2*(1 + c_*(T - t)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
    }

//===========================================================================//
//                               AbcdSquared                                //
//===========================================================================//

    AbcdSquared::AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S)
    : abcd_(new Abcd(a,b,c,d)),
      T_(T), S_(S) {}

    Real AbcdSquared::operator()(Time t) const {
        return abcd_->covariance(t, T_, S_);
    }
}