#include "seal/seal.h"
#include "performance_test.h"

using namespace std;
using namespace seal;

/*
In this file, we have defined all the tests that we have performed in this thesis
"Performance Analysis of FHE Libraries"
*/

void test_sign_algo1()
{
    EncryptionParameters parms(scheme_type::BFV);
    cout << " Pick an encoder to test the sign function. " << endl;
    cout << "  **************************************** " << endl;
    cout << " Enter 1 for integer encoder " << endl;
    cout << " Enter 2 for batch encoder " << endl;
    cout << endl;
    cout << "  **************************************** " << endl;
    int pick = 0;
    cin >> pick;

    if (!(pick))
    {
        cout << "Invalid option." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    if (pick = 1)
    {
        cout << endl;
        size_t poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        auto context = SEALContext::Create(parms);
        sign1(context, pick);

        cout << endl;
        poly_modulus_degree = 16384;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        context = SEALContext::Create(parms);
        sign1(context, pick);

        cout << endl;
        poly_modulus_degree = 32768;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        context = SEALContext::Create(parms);
        sign1(context, pick);
    }
    else if (pick = 2)
    {
        size_t poly_modulus_degree = 4096;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        auto context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 16384;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 32768;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);
    }
}

void test_sign_algo2()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    auto context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);
}

void test_sigmoid()
{
    EncryptionParameters parms(scheme_type::CKKS);

    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 24, 24, 24, 24, 24, 24, 24, 24, 24 }));
    auto context = SEALContext::Create(parms);
    int q = 24;
    sigmoid_setup(context, q);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 40, 40, 40, 40, 40, 40, 40, 40, 40 }));
    context = SEALContext::Create(parms);
    q = 40;
    sigmoid_setup(context, q);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60, 60, 60 }));
    context = SEALContext::Create(parms);
    q = 60;
    sigmoid_setup(context, q);
}

void test_dnn()
{
    EncryptionParameters parms(scheme_type::CKKS);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(
        poly_modulus_degree, { 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25 }));
    auto context = SEALContext::Create(parms);
    int q = 25;
    dnn(context, q);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(
        poly_modulus_degree, { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 }));

    context = SEALContext::Create(parms);
    q = 50;
    dnn(context, q);
}

void test_discreteNN()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    auto context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);
}



int main()
{
    cout << "Microsoft SEAL version: " << SEAL_VERSION << endl;
    cout << endl;

    while (true)
    {
        cout << " ---------------------------------------------" << endl;
        cout << " Select the test to run out of below options" << endl;
        cout << " 1. sign function with algorithm1 " << endl;
        cout << " 2. sign function with algorithm2 " << endl;
        cout << " 3. sigmoid function " << endl;
        cout << " 4. deep neural network " << endl;
        cout << " 5. discrete neural network " << endl;
        cout << " --------------------------------------------- " << endl;

        int select;
        cin >> select;
        switch (select)
        {
        case 1:
            test_sign_algo1();
            break;
        case 2:
            test_sign_algo2();
            break;
        case 3:
            test_sigmoid();
            break;
        case 4:
            test_dnn();
            break;
        case 5:
            test_discreteNN();
            break;
        default:
            cout << " Invalid option " << endl;
        }
    }

    return 0;
}
