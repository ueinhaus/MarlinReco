#ifndef JAMA_EIG_H
#define JAMA_EIG_H


#include "tnt_math_utils.h"

#include <algorithm>
// for min(), max() below



using namespace std;

namespace JAMMA
{
class Eigenvalue
{
 private:

   /** Row and column dimension (square matrix).  */
   int _n{};
   int issymmetric{}; /* boolean*/
   float d[3]{};
   float e[3]{};
   float V[3][3]{};
   float H[3][3]{};
   float ort[3]{};
   
   void tred2() {

   //  This is derived from the Algol procedures tred2 by
   //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
   //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
   //  Fortran subroutine in EISPACK.

      for (int j = 0; j < _n; j++) {
         d[j] = V[_n-1][j];
      }

      // Householder reduction to tridiagonal form.
   
      for (int i = _n-1; i > 0; i--) {
   
         // Scale to avoid under/overflow.
   
         float scale = 0.0;
         float h = 0.0;
         for (int k = 0; k < i; k++) {
            scale = scale + abs(d[k]);
         }
         if (scale == 0.0) {
            e[i] = d[i-1];
            for (int j = 0; j < i; j++) {
               d[j] = V[i-1][j];
               V[i][j] = 0.0;
               V[j][i] = 0.0;
            }
         } else {
   
            // Generate Householder vector.
   
            for (int k = 0; k < i; k++) {
               d[k] /= scale;
               h += d[k] * d[k];
            }
            float f = d[i-1];
            float g = sqrt(h);
            if (f > 0) {
               g = -g;
            }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (int j = 0; j < i; j++) {
               e[j] = 0.0;
            }
   
            // Apply similarity transformation to remaining columns.
   
            for (int j = 0; j < i; j++) {
               f = d[j];
               V[j][i] = f;
               g = e[j] + V[j][j] * f;
               for (int k = j+1; k <= i-1; k++) {
                  g += V[k][j] * d[k];
                  e[k] += V[k][j] * f;
               }
               e[j] = g;
            }
            f = 0.0;
            for (int j = 0; j < i; j++) {
               e[j] /= h;
               f += e[j] * d[j];
            }
            float hh = f / (h + h);
            for (int j = 0; j < i; j++) {
               e[j] -= hh * d[j];
            }
            for (int j = 0; j < i; j++) {
               f = d[j];
               g = e[j];
               for (int k = j; k <= i-1; k++) {
                  V[k][j] -= (f * e[k] + g * d[k]);
               }
               d[j] = V[i-1][j];
               V[i][j] = 0.0;
            }
         }
         d[i] = h;
      }
   
      // Accumulate transformations.
   
      for (int i = 0; i < _n-1; i++) {
         V[_n-1][i] = V[i][i];
         V[i][i] = 1.0;
         float h = d[i+1];
         if (h != 0.0) {
            for (int k = 0; k <= i; k++) {
               d[k] = V[k][i+1] / h;
            }
            for (int j = 0; j <= i; j++) {
               float g = 0.0;
               for (int k = 0; k <= i; k++) {
                  g += V[k][i+1] * V[k][j];
               }
               for (int k = 0; k <= i; k++) {
                  V[k][j] -= g * d[k];
               }
            }
         }
         for (int k = 0; k <= i; k++) {
            V[k][i+1] = 0.0;
         }
      }
      for (int j = 0; j < _n; j++) {
         d[j] = V[_n-1][j];
         V[_n-1][j] = 0.0;
      }
      V[_n-1][_n-1] = 1.0;
      e[0] = 0.0;
   } 

   // Symmetric tridiagonal QL algorithm.
   
   void tql2 () {

   //  This is derived from the Algol procedures tql2, by
   //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
   //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
   //  Fortran subroutine in EISPACK.
   
      for (int i = 1; i < _n; i++) {
         e[i-1] = e[i];
      }
      e[_n-1] = 0.0;
   
      float f = 0.0;
      float tst1 = 0.0;
      float eps = pow(2.0,-52.0);
      for (int l = 0; l < _n; l++) {

         // Find small subdiagonal element
   
         tst1 = max(tst1,abs(d[l]) + abs(e[l]));
         int m = l;

        // Original while-loop from Java code
         while (m < _n) {
            if (abs(e[m]) <= eps*tst1) {
               break;
            }
            m++;
         }

   
         // If m == l, d[l] is an eigenvalue,
         // otherwise, iterate.
   
         if (m > l) {
            int iter = 0;
            do {
               iter = iter + 1;  // (Could check iteration count here.)
   
               // Compute implicit shift
   
               float g = d[l];
               float p = (d[l+1] - g) / (2.0 * e[l]);
               float r = sqrt( p*p + 1 ); //hypot(p,1.0);
               if (p < 0) {
                  r = -r;
               }
               d[l] = e[l] / (p + r);
               d[l+1] = e[l] * (p + r);
               float dl1 = d[l+1];
               float h = g - d[l];
               for (int i = l+2; i < _n; i++) {
                  d[i] -= h;
               }
               f = f + h;
   
               // Implicit QL transformation.
   
               p = d[m];
               float c = 1.0;
               float c2 = c;
               float c3 = c;
               float el1 = e[l+1];
               float s = 0.0;
               float s2 = 0.0;
               for (int i = m-1; i >= l; i--) {
                  c3 = c2;
                  c2 = c;
                  s2 = s;
                  g = c * e[i];
                  h = c * p;
                  r = sqrt( p*p + e[i]*e[i] ); //hypot(p,e[i]);
                  e[i+1] = s * r;
                  s = e[i] / r;
                  c = p / r;
                  p = c * d[i] - s * g;
                  d[i+1] = h + s * (c * g + s * d[i]);
   
                  // Accumulate transformation.
   
                  for (int k = 0; k < _n; k++) {
                     h = V[k][i+1];
                     V[k][i+1] = s * V[k][i] + c * h;
                     V[k][i] = c * V[k][i] - s * h;
                  }
               }
               p = -s * s2 * c3 * el1 * e[l] / dl1;
               e[l] = s * p;
               d[l] = c * p;
   
               // Check for convergence.
   
            } while (abs(e[l]) > eps*tst1);
         }
         d[l] = d[l] + f;
         e[l] = 0.0;
      }
     
      // Sort eigenvalues and corresponding vectors.
   
      for (int i = 0; i < _n-1; i++) {
         int k = i;
         float p = d[i];
         for (int j = i+1; j < _n; j++) {
            if (d[j] < p) {
               k = j;
               p = d[j];
            }
         }
         if (k != i) {
            d[k] = d[i];
            d[i] = p;
            for (int j = 0; j < _n; j++) {
               p = V[j][i];
               V[j][i] = V[j][k];
               V[j][k] = p;
            }
         }
      }
   }

   // Nonsymmetric reduction to Hessenberg form.

   void orthes () {
   
      //  This is derived from the Algol procedures orthes and ortran,
      //  by Martin and Wilkinson, Handbook for Auto. Comp.,
      //  Vol.ii-Linear Algebra, and the corresponding
      //  Fortran subroutines in EISPACK.
   
      int low = 0;
      int high = _n-1;
   
      for (int m = low+1; m <= high-1; m++) {
   
         // Scale column.
   
         float scale = 0.0;
         for (int i = m; i <= high; i++) {
            scale = scale + abs(H[i][m-1]);
         }
         if (scale != 0.0) {
   
            // Compute Householder transformation.
   
            float h = 0.0;
            for (int i = high; i >= m; i--) {
               ort[i] = H[i][m-1]/scale;
               h += ort[i] * ort[i];
            }
            float g = sqrt(h);
            if (ort[m] > 0) {
               g = -g;
            }
            h = h - ort[m] * g;
            ort[m] = ort[m] - g;
   
            // Apply Householder similarity transformation
            // H = (I-u*u'/h)*H*(I-u*u')/h)
   
            for (int j = m; j < _n; j++) {
               float f = 0.0;
               for (int i = high; i >= m; i--) {
                  f += ort[i]*H[i][j];
               }
               f = f/h;
               for (int i = m; i <= high; i++) {
                  H[i][j] -= f*ort[i];
               }
           }
   
           for (int i = 0; i <= high; i++) {
               float f = 0.0;
               for (int j = high; j >= m; j--) {
                  f += ort[j]*H[i][j];
               }
               f = f/h;
               for (int j = m; j <= high; j++) {
                  H[i][j] -= f*ort[j];
               }
            }
            ort[m] = scale*ort[m];
            H[m][m-1] = scale*g;
         }
      }
   
      // Accumulate transformations (Algol's ortran).

      for (int i = 0; i < _n; i++) {
         for (int j = 0; j < _n; j++) {
            V[i][j] = (i == j ? 1.0 : 0.0);
         }
      }

      for (int m = high-1; m >= low+1; m--) {
         if (H[m][m-1] != 0.0) {
            for (int i = m+1; i <= high; i++) {
               ort[i] = H[i][m-1];
            }
            for (int j = m; j <= high; j++) {
               float g = 0.0;
               for (int i = m; i <= high; i++) {
                  g += ort[i] * V[i][j];
               }
               // Float division avoids possible underflow
               g = (g / ort[m]) / H[m][m-1];
               for (int i = m; i <= high; i++) {
                  V[i][j] += g * ort[i];
               }
            }
         }
      }
   }


   // Complex scalar division.

   float cdivr{}, cdivi{};
   void cdiv(float xr, float xi, float yr, float yi) {
      float r,dd;
      if (abs(yr) > abs(yi)) {
         r = yi/yr;
         dd = yr + r*yi;
         cdivr = (xr + r*xi)/dd;
         cdivi = (xi - r*xr)/dd;
      } else {
         r = yr/yi;
         dd = yi + r*yr;
         cdivr = (r*xr + xi)/dd;
         cdivi = (r*xi - xr)/dd;
      }
   }


   // Nonsymmetric reduction from Hessenberg to real Schur form.

   void hqr2 () {
   
      //  This is derived from the Algol procedure hqr2,
      //  by Martin and Wilkinson, Handbook for Auto. Comp.,
      //  Vol.ii-Linear Algebra, and the corresponding
      //  Fortran subroutine in EISPACK.
   
      // Initialize
   
      int nn = 3;
      int n = nn-1;
      int low = 0;
      int high = nn-1;
      float eps = pow(2.0,-52.0);
      float exshift = 0.0;
      float p=0,q=0,r=0,s=0,z=0,t,w,x,y;
   
      // Store roots isolated by balanc and compute matrix norm
   
      float norm = 0.0;
      for (int i = 0; i < nn; i++) {
         if ((i < low) || (i > high)) {
            d[i] = H[i][i];
            e[i] = 0.0;
         }
         for (int j = max(i-1,0); j < nn; j++) {
            norm = norm + abs(H[i][j]);
         }
      }
   
      // Outer loop over eigenvalue index
   
      int iter = 0;
      while (n >= low) {
   
         // Look for single small sub-diagonal element
   
         int l = n;
         while (l > low) {
            s = abs(H[l-1][l-1]) + abs(H[l][l]);
            if (s == 0.0) {
               s = norm;
            }
            if (abs(H[l][l-1]) < eps * s) {
               break;
            }
            l--;
         }
       
         // Check for convergence
         // One root found
   
         if (l == n) {
            H[n][n] = H[n][n] + exshift;
            d[n] = H[n][n];
            e[n] = 0.0;
            n--;
            iter = 0;
   
         // Two roots found
   
         } else if (l == n-1) {
            w = H[n][n-1] * H[n-1][n];
            p = (H[n-1][n-1] - H[n][n]) / 2.0;
            q = p * p + w;
            z = sqrt(abs(q));
            H[n][n] = H[n][n] + exshift;
            H[n-1][n-1] = H[n-1][n-1] + exshift;
            x = H[n][n];
   
            // float pair
   
            if (q >= 0) {
               if (p >= 0) {
                  z = p + z;
               } else {
                  z = p - z;
               }
               d[n-1] = x + z;
               d[n] = d[n-1];
               if (z != 0.0) {
                  d[n] = x - w / z;
               }
               e[n-1] = 0.0;
               e[n] = 0.0;
               x = H[n][n-1];
               s = abs(x) + abs(z);
               p = x / s;
               q = z / s;
               r = sqrt(p * p+q * q);
               p = p / r;
               q = q / r;
   
               // Row modification
   
               for (int j = n-1; j < nn; j++) {
                  z = H[n-1][j];
                  H[n-1][j] = q * z + p * H[n][j];
                  H[n][j] = q * H[n][j] - p * z;
               }
   
               // Column modification
   
               for (int i = 0; i <= n; i++) {
                  z = H[i][n-1];
                  H[i][n-1] = q * z + p * H[i][n];
                  H[i][n] = q * H[i][n] - p * z;
               }
   
               // Accumulate transformations
   
               for (int i = low; i <= high; i++) {
                  z = V[i][n-1];
                  V[i][n-1] = q * z + p * V[i][n];
                  V[i][n] = q * V[i][n] - p * z;
               }
   
            // Complex pair
   
            } else {
               d[n-1] = x + p;
               d[n] = x + p;
               e[n-1] = z;
               e[n] = -z;
            }
            n = n - 2;
            iter = 0;
   
         // No convergence yet
   
         } else {
   
            // Form shift
   
            x = H[n][n];
            y = 0.0;
            w = 0.0;
            if (l < n) {
               y = H[n-1][n-1];
               w = H[n][n-1] * H[n-1][n];
            }
   
            // Wilkinson's original ad hoc shift
   
            if (iter == 10) {
               exshift += x;
               for (int i = low; i <= n; i++) {
                  H[i][i] -= x;
               }
               s = abs(H[n][n-1]) + abs(H[n-1][n-2]);
               x = y = 0.75 * s;
               w = -0.4375 * s * s;
            }

            // MATLAB's new ad hoc shift

            if (iter == 30) {
                s = (y - x) / 2.0;
                s = s * s + w;
                if (s > 0) {
                    s = sqrt(s);
                    if (y < x) {
                       s = -s;
                    }
                    s = x - w / ((y - x) / 2.0 + s);
                    for (int i = low; i <= n; i++) {
                       H[i][i] -= s;
                    }
                    exshift += s;
                    x = y = w = 0.964;
                }
            }
   
            iter = iter + 1;   // (Could check iteration count here.)
   

// for abs() below
            // Look for two consecutive small sub-diagonal elements
   
            int m = n-2;
            while (m >= l) {
               z = H[m][m];
               r = x - z;
               s = y - z;
               p = (r * s - w) / H[m+1][m] + H[m][m+1];
               q = H[m+1][m+1] - z - r - s;
               r = H[m+2][m+1];
               s = abs(p) + abs(q) + abs(r);
               p = p / s;
               q = q / s;

// for abs() below
               r = r / s;
               if (m == l) {
                  break;
               }
               if (abs(H[m][m-1]) * (abs(q) + abs(r)) <
                  eps * (abs(p) * (abs(H[m-1][m-1]) + abs(z) +
                  abs(H[m+1][m+1])))) {
                     break;
               }
               m--;
            }
   
            for (int i = m+2; i <= n; i++) {
               H[i][i-2] = 0.0;
               if (i > m+2) {
                  H[i][i-3] = 0.0;
               }
            }
   
            // Float QR step involving rows l:n and columns m:n
   
            for (int k = m; k <= n-1; k++) {
               int notlast = (k != n-1);
               if (k != m) {
                  p = H[k][k-1];
                  q = H[k+1][k-1];
                  r = (notlast ? H[k+2][k-1] : 0.0);
                  x = abs(p) + abs(q) + abs(r);
                  if (x != 0.0) {
                     p = p / x;
                     q = q / x;
                     r = r / x;
                  }
               }
               if (x == 0.0) {
                  break;
               }
               s = sqrt(p * p + q * q + r * r);
               if (p < 0) {
                  s = -s;
               }
               if (s != 0) {
                  if (k != m) {
                     H[k][k-1] = -s * x;
                  } else if (l != m) {
                     H[k][k-1] = -H[k][k-1];
                  }
                  p = p + s;
                  x = p / s;
                  y = q / s;
                  z = r / s;
                  q = q / p;
                  r = r / p;
   
                  // Row modification
   
                  for (int j = k; j < nn; j++) {
                     p = H[k][j] + q * H[k+1][j];
                     if (notlast) {
                        p = p + r * H[k+2][j];
                        H[k+2][j] = H[k+2][j] - p * z;
                     }
                     H[k][j] = H[k][j] - p * x;
                     H[k+1][j] = H[k+1][j] - p * y;
                  }
   
                  // Column modification
   
                  for (int i = 0; i <= min(n,k+3); i++) {
                     p = x * H[i][k] + y * H[i][k+1];
                     if (notlast) {
                        p = p + z * H[i][k+2];
                        H[i][k+2] = H[i][k+2] - p * r;
                     }
                     H[i][k] = H[i][k] - p;
                     H[i][k+1] = H[i][k+1] - p * q;
                  }
   
                  // Accumulate transformations
   
                  for (int i = low; i <= high; i++) {
                     p = x * V[i][k] + y * V[i][k+1];
                     if (notlast) {
                        p = p + z * V[i][k+2];
                        V[i][k+2] = V[i][k+2] - p * r;
                     }
                     V[i][k] = V[i][k] - p;
                     V[i][k+1] = V[i][k+1] - p * q;
                  }
               }  // (s != 0)
            }  // k loop
         }  // check convergence
      }  // while (n >= low)
      
      // Backsubstitute to find vectors of upper triangular form

      if (norm == 0.0) {
         return;
      }
   
      for (n = nn-1; n >= 0; n--) {
         p = d[n];
         q = e[n];
   
         // float vector
   
         if (q == 0) {
            int l = n;
            H[n][n] = 1.0;
            for (int i = n-1; i >= 0; i--) {
               w = H[i][i] - p;
               r = 0.0;
               for (int j = l; j <= n; j++) {
                  r = r + H[i][j] * H[j][n];
               }
               if (e[i] < 0.0) {
                  z = w;
                  s = r;
               } else {
                  l = i;
                  if (e[i] == 0.0) {
                     if (w != 0.0) {
                        H[i][n] = -r / w;
                     } else {
                        H[i][n] = -r / (eps * norm);
                     }
   
                  // Solve real equations
   
                  } else {
                     x = H[i][i+1];
                     y = H[i+1][i];
                     q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
                     t = (x * s - z * r) / q;
                     H[i][n] = t;
                     if (abs(x) > abs(z)) {
                        H[i+1][n] = (-r - w * t) / x;
                     } else {
                        H[i+1][n] = (-s - y * t) / z;
                     }
                  }
   
                  // Overflow control
   
                  t = abs(H[i][n]);
                  if ((eps * t) * t > 1) {
                     for (int j = i; j <= n; j++) {
                        H[j][n] = H[j][n] / t;
                     }
                  }
               }
            }
   
         // Complex vector
   
         } else if (q < 0) {
            int l = n-1;

            // Last vector component imaginary so matrix is triangular
   
            if (abs(H[n][n-1]) > abs(H[n-1][n])) {
               H[n-1][n-1] = q / H[n][n-1];
               H[n-1][n] = -(H[n][n] - p) / H[n][n-1];
            } else {
               cdiv(0.0,-H[n-1][n],H[n-1][n-1]-p,q);
               H[n-1][n-1] = cdivr;
               H[n-1][n] = cdivi;
            }
            H[n][n-1] = 0.0;
            H[n][n] = 1.0;
            for (int i = n-2; i >= 0; i--) {
               float ra,sa,vr,vi;
               ra = 0.0;
               sa = 0.0;
               for (int j = l; j <= n; j++) {
                  ra = ra + H[i][j] * H[j][n-1];
                  sa = sa + H[i][j] * H[j][n];
               }
               w = H[i][i] - p;
   
               if (e[i] < 0.0) {
                  z = w;
                  r = ra;
                  s = sa;
               } else {
                  l = i;
                  if (e[i] == 0) {
                     cdiv(-ra,-sa,w,q);
                     H[i][n-1] = cdivr;
                     H[i][n] = cdivi;
                  } else {
   
                     // Solve complex equations
   
                     x = H[i][i+1];
                     y = H[i+1][i];
                     vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
                     vi = (d[i] - p) * 2.0 * q;
                     if ((vr == 0.0) && (vi == 0.0)) {
                        vr = eps * norm * (abs(w) + abs(q) +
                        abs(x) + abs(y) + abs(z));
                     }
                     cdiv(x*r-z*ra+q*sa,x*s-z*sa-q*ra,vr,vi);
                     H[i][n-1] = cdivr;
                     H[i][n] = cdivi;
                     if (abs(x) > (abs(z) + abs(q))) {
                        H[i+1][n-1] = (-ra - w * H[i][n-1] + q * H[i][n]) / x;
                        H[i+1][n] = (-sa - w * H[i][n] - q * H[i][n-1]) / x;
                     } else {
                        cdiv(-r-y*H[i][n-1],-s-y*H[i][n],z,q);
                        H[i+1][n-1] = cdivr;
                        H[i+1][n] = cdivi;
                     }
                  }
   
                  // Overflow control

                  t = max(abs(H[i][n-1]),abs(H[i][n]));
                  if ((eps * t) * t > 1) {
                     for (int j = i; j <= n; j++) {
                        H[j][n-1] = H[j][n-1] / t;
                        H[j][n] = H[j][n] / t;
                     }
                  }
               }
            }
         }
      }
   
      // Vectors of isolated roots
   
      for (int i = 0; i < nn; i++) {

// for abs() below
         if (i < low || i > high) {
            for (int j = i; j < nn; j++) {
               V[i][j] = H[i][j];
            }
         }
      }
   
      // Back transformation to get eigenvectors of original matrix
   
      for (int j = nn-1; j >= low; j--) {
         for (int i = low; i <= high; i++) {
            z = 0.0;
            for (int k = low; k <= min(j,high); k++) {
               z = z + V[i][k] * H[k][j];
            }
            V[i][j] = z;
         }
      }
   }

public:

   ~Eigenvalue(){
    
   }
   /** Check for symmetry, then construct the eigenvalue decomposition
   @param A    Square real (non-complex) matrix
   */

   Eigenvalue( float A[3][3]) {
      _n = 3;
      issymmetric = 1;

      for (int j = 0; (j < _n) && issymmetric; j++) {
	  d[j]=0.0;
          e[j]=0.0;
         for (int i = 0; (i < _n) && issymmetric; i++) {
            issymmetric = (A[i][j] == A[j][i]);
        
         }
      }

      if (issymmetric) {
         for (int i = 0; i < _n; i++) {
            for (int j = 0; j < _n; j++) {
               V[i][j] = A[i][j];
            }
         }
   
         // Tridiagonalize.
         tred2();
   
         // Diagonalize.
         tql2();

      } else {
        
         
         for (int j = 0; j < _n; j++) {
            for (int i = 0; i < _n; i++) {
               H[i][j] = A[i][j];
            }
         }
   
         // Reduce to Hessenberg form.
         orthes();
   
         // Reduce Hessenberg to real Schur form.
         hqr2();
      }
   }


   /** Return the eigenvector matrix
   @return     V
   */

   void getV (float V_[3][3]) {
       for (int i=0;i<3;i++)
       {
	   for (int j=0;j<3;j++)
	   { V_[i][j] = V[i][j];}}
      return;
   }

   /** Return the real parts of the eigenvalues
   @return     real(diag(D))
   */

   void getRealEigenvalues (float d_[3]) {
      for (int i=0;i<3;i++)
         { d_[i] = d[i];};
      return ;
   }

   /** Return the imaginary parts of the eigenvalues
   in parameter e_.

   @pararm e_: new matrix with imaginary parts of the eigenvalues.
   */
   void getImagEigenvalues (float e_[3]) {
       for (int i=0;i<3;i++)
         { e_[i] = e[i];}
      return;
   }

   
/** 
	Computes the block diagonal eigenvalue matrix.
    If the original matrix A is not symmetric, then the eigenvalue 
	matrix D is block diagonal with the real eigenvalues in 1-by-1 
	blocks and any complex eigenvalues,
    a + i*b, in 2-by-2 blocks, [a, b; -b, a].  That is, if the complex
    eigenvalues look like
<pre>

          u + iv     .        .          .      .    .
            .      u - iv     .          .      .    .
            .        .      a + ib       .      .    .
            .        .        .        a - ib   .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
        then D looks like
<pre>

            u        v        .          .      .    .
           -v        u        .          .      .    . 
            .        .        a          b      .    .
            .        .       -b          a      .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
    This keeps V a real matrix in both symmetric and non-symmetric
    cases, and A*V = V*D.

	@param D: upon return, the matrix is filled with the block diagonal 
	eigenvalue matrix.
	
*/
   void getD (float D[3][3]) {
      for (int i = 0; i < _n; i++) {
         for (int j = 0; j < _n; j++) {
            D[i][j] = 0.0;
         }
         D[i][i] = d[i];
         if (e[i] > 0) {
            D[i][i+1] = e[i];
         } else if (e[i] < 0) {
            D[i][i-1] = e[i];
         }
      }
   }
};


}

#endif
// JAMA_EIG_H
