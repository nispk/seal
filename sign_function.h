#include "math.h"
#include "seal/seal.h"
#include <iostream>
using namespace std;
using namespace seal;


inline vector<Ciphertext> rotate_right(vector<Ciphertext> x, int64_t s)
{
    for (int64_t i = 0; i < s; i++)
    {
        x.insert(x.begin(), x.back());
        x.pop_back();
    }
    return x;
}

inline void algo1_sign(
    shared_ptr<SEALContext> context, Ciphertext input, vector<Ciphertext> func_enc, vector<Plaintext> func_indices,
    Ciphertext &num_output, Ciphertext &den_output, int64_t lookup_size, RelinKeys rk)
{
    
    Evaluator evaluator(context);
    IntegerEncoder intencoder(context);
    int64_t N = lookup_size;
    int64_t n = static_cast<int64_t>(log2(N));
    int64_t max_input_range = N / 2;
    vector<Ciphertext> x_enc;
    vector<Ciphertext> rot_x;
    vector<Ciphertext> numerator;
    Ciphertext temp2, product;

    for (int j = 0; j <= N; j++)
    {
        x_enc.push_back(input);
        evaluator.sub_plain_inplace(x_enc[j], func_indices[j]);
    }

    for (size_t s = 0; s < n; s++)
    {
        rot_x.clear();
        rot_x = rotate_right(x_enc, pow(2, s));
        for (int i = 0; i <= N; i++)
        {
            evaluator.multiply(x_enc[i], rot_x[i], temp2);
            evaluator.relinearize_inplace(temp2, rk);
            x_enc[i] = temp2;
        }
    }

    for (int p = 0; p <= N; p++)
    {
        evaluator.multiply(x_enc[p], func_enc[p], product);
        evaluator.relinearize_inplace(product, rk);
        numerator.push_back(product);
    }

    for (int q = 1; q <= N; q++)
    {
        evaluator.add_inplace(x_enc[0], x_enc[q]);
        evaluator.add_inplace(numerator[0], numerator[q]);
    }

    num_output = numerator[0];
    den_output = x_enc[0];

    x_enc.clear();
    rot_x.clear();
    numerator.clear();
    temp2.release();
    
}




inline void algo2_sign(
    shared_ptr<SEALContext> context, Ciphertext input, vector<Ciphertext> func_enc, vector<Plaintext> func_indices, vector<Plaintext> preamble,
    Ciphertext &output, int64_t lookup_size, RelinKeys rk)
{
 
    Evaluator evaluator(context);

    Ciphertext product;
    vector<Ciphertext> x_enc;
    vector<Ciphertext> numerator;

    int64_t N = lookup_size;
    int64_t n = static_cast <int64_t> (log2(N));
    int64_t max_input_range = N/2;
    
    
    for (int64_t j = 0; j <= N; j++)
    {
        x_enc.push_back(input);
        evaluator.sub_plain_inplace(x_enc[j], func_indices[j]);
    }

    vector<Ciphertext> rot_x;
    for (size_t s = 0; s < n; s++)
    {
        rot_x.clear();
        rot_x = rotate_right(x_enc, pow(2, s));
        for (int k = 0; k <= N; k++)
        {
            evaluator.multiply_inplace(x_enc[k], rot_x[k]);
            evaluator.relinearize_inplace(x_enc[k], rk);
        }
    }

    for (int m = 0; m <= N; m++)
    {
        evaluator.multiply_plain_inplace(x_enc[m], preamble[m]);
        evaluator.relinearize_inplace(x_enc[m], rk);
        evaluator.multiply_inplace(x_enc[m], func_enc[m]);
        evaluator.relinearize_inplace(x_enc[m], rk);
    }

    for (int q = 1; q <= N; q++)
    {
        evaluator.add_inplace(x_enc[0], x_enc[q]);
    }


    output = x_enc[0];

    product.release();
    numerator.clear();
    rot_x.clear();
    x_enc.clear();

}
