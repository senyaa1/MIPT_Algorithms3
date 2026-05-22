#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using Vector64 = std::vector<int64_t>;

namespace ModMath
{
	constexpr int64_t MOD = 7340033;
	constexpr int64_t G = 3;

	int64_t power(int64_t base, int64_t exp)
	{
		int64_t res = 1;
		base %= MOD;
		while (exp > 0)
		{
			if (exp % 2 == 1)
			{
				res = (res * base) % MOD;
			}
			base = (base * base) % MOD;
			exp /= 2;
		}
		return res;
	}

	int64_t modInverse(int64_t n)
	{
		return power(n, MOD - 2);
	}
}

namespace FftTransformer
{
	void transform(Vector64 &a, bool invert)
	{
		std::size_t n = a.size();
		for (std::size_t i = 1, j = 0; i < n; ++i)
		{
			std::size_t bit = n >> 1;
			for (; j & bit; bit >>= 1)
			{
				j ^= bit;
			}
			j ^= bit;
			if (i < j)
			{
				std::swap(a[i], a[j]);
			}
		}
		for (std::size_t len = 2; len <= n; len <<= 1)
		{
			int64_t wlen = ModMath::power(ModMath::G, (ModMath::MOD - 1) / static_cast<int64_t>(len));
			if (invert)
			{
				wlen = ModMath::modInverse(wlen);
			}
			for (std::size_t i = 0; i < n; i += len)
			{
				int64_t w = 1;
				for (std::size_t j = 0; j < len / 2; ++j)
				{
					int64_t u = a[i + j];
					int64_t v = (a[i + j + len / 2] * w) % ModMath::MOD;
					a[i + j] = u + v < ModMath::MOD ? u + v : u + v - ModMath::MOD;
					a[i + j + len / 2] = u - v >= 0 ? u - v : u - v + ModMath::MOD;
					w = (w * wlen) % ModMath::MOD;
				}
			}
		}
		if (invert)
		{
			int64_t n_inv = ModMath::modInverse(static_cast<int64_t>(n));
			for (int64_t &x : a)
			{
				x = (x * n_inv) % ModMath::MOD;
			}
		}
	}
}

class Polynomial
{
      private:
	Vector64 coeffs;

      public:
	explicit Polynomial(const Vector64 &c) : coeffs(c)
	{
	}

	explicit Polynomial(std::size_t n) : coeffs(n)
	{
	}

	friend std::istream &operator>>(std::istream &is, Polynomial &p)
	{
		for (auto &coeff : p.coeffs)
		{
			is >> coeff;
		}
		return is;
	}

	friend std::ostream &operator<<(std::ostream &os, const Polynomial &p)
	{
		for (std::size_t i = 0; i < p.coeffs.size(); ++i)
		{
			os << p.coeffs[i] << (i == p.coeffs.size() - 1 ? "" : " ");
		}
		return os;
	}

	int64_t operator[](std::size_t i) const
	{
		return coeffs[i];
	}

	Polynomial inverse(std::size_t m) const
	{
		if (coeffs.empty() || coeffs[0] == 0)
		{
			return Polynomial(Vector64{});
		}
		Vector64 res;
		res.push_back(ModMath::modInverse(coeffs[0]));
		std::size_t len = 1;
		while (len < m)
		{
			len <<= 1;
			Vector64 a_copy(coeffs.begin(), coeffs.begin() + std::min(coeffs.size(), len));
			a_copy.resize(2 * len, 0);
			Vector64 res_copy = res;
			res_copy.resize(2 * len, 0);

			FftTransformer::transform(a_copy, false);
			FftTransformer::transform(res_copy, false);

			for (std::size_t i = 0; i < 2 * len; ++i)
			{
				res_copy[i] = res_copy[i] *
					      (2 - a_copy[i] * res_copy[i] % ModMath::MOD + ModMath::MOD) %
					      ModMath::MOD;
			}

			FftTransformer::transform(res_copy, true);
			res_copy.resize(len);
			res = res_copy;
		}
		res.resize(m);
		return Polynomial(res);
	}
};

int main()
{
	std::size_t m, n;
	std::cin >> m >> n;

	Polynomial poly(n + 1);
	std::cin >> poly;

	if (poly[0] == 0)
	{
		std::cout << "The ears of a dead donkey" << std::endl;
		return 0;
	}

	Polynomial inv = poly.inverse(m);
	std::cout << inv << std::endl;

	return 0;
}
