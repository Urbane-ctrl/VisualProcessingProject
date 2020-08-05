#ifndef VISPRO_H
#define VISPRO_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace std;

float* ud_vector(int n, int m) {
    // function to initiallize a vector. Note that function had to be renamed because of ambiguity in opencv2/calib3d.hpp.
    /* create a vector starting at index n and ending at index m */
    float* vec;

    vec = (float*)malloc((m - n + 1) * sizeof(float));
    for (int i = 0; i < (m - n + 1); i++) {
        vec[i] = 1;
    }
    vec -= n;
    return(vec);
}

float** matrix(int n1, int n2, int m1, int m2) {
    /* create a matrix, for example matrix (1, 3, 1, 4); */
    float** mat;
    int i;

    mat = (float**)malloc((n2 - n1 + 1) * sizeof(float*));

    mat -= n1;

    for (i = n1; i <= n2; i++)
    {
        mat[i] = (float*)malloc((m2 - m1 + 1) * sizeof(float));
        mat[i] -= m1;
    }

    return(mat);
}

static float at, bt, ct;
#define PYTHAG(a,b) ((at=fabs(a)) > (bt=fabs(b)) ? \
(ct=bt/at,at*sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt,bt*sqrt(1.0+ct*ct)): 0.0))

static float maxarg1, maxarg2;
#define MAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
    (maxarg1) : (maxarg2))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

void svdRoutine(float** A, int m, int n, float* w, float** v)
/* Calculates the SVD of matrix A
Inputs:
         A: a matrix of m lines and n columns
     m: the number of lines of matrix A
     n: the number of columns of matrix A
Outputs:
     w: a vector of size n that will contain all eigen values
         v: a matrix of n lines and n columns. It will contain
        the eigen vectors(the columns of this matrix) corresponding
    to the wigen values in w.
            For example, the eigen value at position i in w corresponds to
    the eigenvector at column i.
 */
{
    int flag, i, its, j, jj, k, l, nm;
    float c, f, h, s, x, y, z;
    float anorm = 0.0, g = 0.0, scale = 0.0;
    float* rv1;
    int setBit = 0;

    if (m < n)
        printf("SVDCMP: You must augment A with extra zero rows\n");
    rv1 = ud_vector(1, n);
    for (i = 1; i <= n; i++) {
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i <= m) {
            for (k = i; k <= m; k++) {
                scale += fabs(A[k][i]);
            }
            if (scale) {
                for (k = i; k <= m; k++) {
                    A[k][i] /= scale;
                    s += A[k][i] * A[k][i];
                }
                f = A[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                A[i][i] = f - g;
                if (i != n) {
                    for (j = l; j <= n; j++) {
                        for (s = 0.0, k = i; k <= m; k++) s += A[k][i] * A[k][j];
                        f = s / h;
                        for (k = i; k <= m; k++) A[k][j] += f * A[k][i];
                    }
                }
                for (k = i; k <= m; k++) A[k][i] *= scale;
            }
        }
        w[i] = scale * g;
        g = s = scale = 0.0;
        if (i <= m && i != n) {
            for (k = l; k <= n; k++) scale += fabs(A[i][k]);
            if (scale) {
                for (k = l; k <= n; k++) {
                    A[i][k] /= scale;
                    s += A[i][k] * A[i][k];
                }
                f = A[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                A[i][l] = f - g;
                for (k = l; k <= n; k++) rv1[k] = A[i][k] / h;
                if (i != m) {
                    for (j = l; j <= m; j++) {
                        for (s = 0.0, k = l; k <= n; k++) s += A[j][k] * A[i][k];
                        for (k = l; k <= n; k++) A[j][k] += s * rv1[k];
                    }
                }
                for (k = l; k <= n; k++) A[i][k] *= scale;
            }
        }
        anorm = MAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
    }
    for (i = n; i >= 1; i--) {
        if (i < n) {
            if (g) {
                for (j = l; j <= n; j++)
                    v[j][i] = (A[i][j] / A[i][l]) / g;
                for (j = l; j <= n; j++) {
                    for (s = 0.0, k = l; k <= n; k++) s += A[i][k] * v[k][j];
                    for (k = l; k <= n; k++) v[k][j] += s * v[k][i];
                }
            }
            for (j = l; j <= n; j++) v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
    for (i = n; i >= 1; i--) {
        l = i + 1;
        g = w[i];
        if (i < n)
            for (j = l; j <= n; j++) A[i][j] = 0.0;
        if (g) {
            g = 1.0 / g;
            if (i != n) {
                for (j = l; j <= n; j++) {
                    for (s = 0.0, k = l; k <= m; k++) s += A[k][i] * A[k][j];
                    f = (s / A[i][i]) * g;
                    for (k = i; k <= m; k++) A[k][j] += f * A[k][i];
                }
            }
            for (j = i; j <= m; j++) A[j][i] *= g;
        }
        else {
            for (j = i; j <= m; j++) A[j][i] = 0.0;
        }
        ++A[i][i];
    }
    for (k = n; k >= 1; k--) {
        for (its = 1; its <= 30; its++) {
            flag = 1;
            for (l = k; l >= 1; l--) {
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) {
                    flag = 0;
                    break;
                }
                if (fabs(w[nm]) + anorm == anorm) break;
            }
            if (flag) {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) {
                        g = w[i];
                        h = PYTHAG(f, g);
                        w[i] = h;
                        h = 1.0 / h;
                        c = g * h;
                        s = (-f * h);
                        for (j = 1; j <= m; j++) {
                            y = A[j][nm];
                            z = A[j][i];
                            A[j][nm] = y * c + z * s;
                            A[j][i] = z * c - y * s;
                        }
                    }
                }
            }
            z = w[k];
            if (l == k) {
                if (z < 0.0) {
                    w[k] = -z;
                    for (j = 1; j <= n; j++) v[j][k] = (-v[j][k]);
                }
                break;
            }
            if (its == 30) printf("No convergence in 30 SVDCMP iterations\n");
            x = w[l];
            nm = k - 1;
            y = w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
            c = s = 1.0;
            for (j = l; j <= nm; j++) {
                i = j + 1;
                g = rv1[i];
                y = w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 1; jj <= n; jj++) {
                    x = v[jj][j];
                    z = v[jj][i];
                    v[jj][j] = x * c + z * s;
                    v[jj][i] = z * c - x * s;
                }
                z = PYTHAG(f, h);
                w[j] = z;
                if (z) {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 1; jj <= m; jj++) {
                    y = A[jj][j];
                    z = A[jj][i];
                    A[jj][j] = y * c + z * s;
                    A[jj][i] = z * c - y * s;
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = x;
        }
    }
}


#undef SIGN
#undef MAX
#undef PYTHAG

#endif // VISPRO_H