#ifndef UTILS_H
#define UTILS_H

int equal_portions(int total)
{
	int parts = 1;
	while (true)
	{
		int sum = 0;

		for (int i = 1; i <= parts; i++)
		{
			sum += i;
		}
		std::cout << "parts: " << parts << ", " << sum << std::endl;
		if (sum >= total)
			return parts;
		parts++;
	}
}
#endif // !UTILS_H
