
#define LIMITER_FILTER_SIZE		64
#define LIMITER_FILTER_MASK		(LIMITER_FILTER_SIZE - 1)

static float LimiterFilter0[LIMITER_FILTER_SIZE/2] = { 0 };
static float LimiterFilter1[LIMITER_FILTER_SIZE/2] = { 0 };
static float LimiterFilter2[LIMITER_FILTER_SIZE/4] = { 0 };
static float LimiterFilter3[LIMITER_FILTER_SIZE/4] = { 0 };

struct sample
{
	float L;
	float R;
};

class Limiter
{
private:
	void CreateFilter(float *p, int size, int shape)
	{
		float sum = 0;

		for (int i = 0; i < size/2; i++)
		{
			if (shape == 0)
				p[i] = (float)sin((i + 1) * M_PI / (size+1));
			else if (shape == 1)
				p[i] = (float)(i + 1);

			sum += p[i];
		}

		for (int i = 0; i < size/2; i++)
			p[i] /= 2 * sum;
	}

public:
	Limiter()
	{
		if (LimiterFilter0[0] == 0.0f)
		{
			CreateFilter(LimiterFilter0, LIMITER_FILTER_SIZE, 0);
			CreateFilter(LimiterFilter1, LIMITER_FILTER_SIZE, 1);
			CreateFilter(LimiterFilter2, LIMITER_FILTER_SIZE/2, 0);
			CreateFilter(LimiterFilter3, LIMITER_FILTER_SIZE/2, 1);

		}

		SetCeiling(1.0f);
		SetThreshold(0.1f);
		gain = 1.0f;
		holdcount = 0;
		fipos = 0;
		inpos = 0;
		rstep = 0.0f;
		rstepexp = 1.0f;
		rtime = 1000;
		mingain = 1.0f;
		lookahead = 127;
		releaseshape = 0;

		filtertype = -1;
		SetFilterType(0);
	}

	void SetCeiling(float x)
	{
		ceiling = x * 32767.0f;
	}

	void SetThreshold(float x)
	{
		threshold = x * 32767.0f;
	}

	void SetRelease(float x)
	{
		rtime = max(1, (int)x);
	}

	void SetFilterType(int x)
	{
		if (x == filtertype)
			return;

		filtertype = x;

		switch(x)
		{
		case 0: pfilter = LimiterFilter0; break;
		case 1:	pfilter = LimiterFilter1; break;
		case 2:	pfilter = LimiterFilter2; break;
		case 3:	pfilter = LimiterFilter3; break;
		case 4:	pfilter = LimiterFilter0; break;
		case 5:	pfilter = LimiterFilter0; break;
		default: assert(false);
		}

		switch(x)
		{
		case 0:
		case 1:
		case 4:
		case 5:
			filtersize = LIMITER_FILTER_SIZE;
			filtermask = LIMITER_FILTER_SIZE - 1;
			break;
		case 2:
		case 3:
			filtersize = LIMITER_FILTER_SIZE / 2;
			filtermask = LIMITER_FILTER_SIZE / 2 - 1;
			break;
		default: assert(false);
		}

		for (int i = 0; i < LIMITER_FILTER_SIZE; i++)
		{
			fi[i] = 0.0f;
			inputs[i].L = 0.0f;
			inputs[i].R = 0.0f;
		}

	}

	void SetLookahead(int x)
	{
		lookahead = x;
	}

	void SetReleaseShape(int x)
	{
		releaseshape = x;
	}

	float *GetMinGainPtr() { return &mingain; }
	int GetLatency() { return lookahead * (filtersize - 1) / 127; }

	void Process(sample *psamples, int numsamples)
	{
		float outgain = ceiling / threshold;
		
		int ioffset = (filtersize - 1) - lookahead * (filtersize - 1) / 127;

		for (int i = 0; i < numsamples; i++)
		{
			sample in = psamples[i];
			float sum;

			if (filtertype == 5)
			{
				sum = fi[fipos & filtermask];	// no filter
			}
			else
			{
				sum = 0.0f;
	
				for (int j = 0; j < filtersize/2; j++)
					sum += pfilter[j] * (fi[(fipos-1-j) & filtermask] + fi[(fipos + j) & filtermask]);
			}

			if (filtertype == 4)
			{
				psamples[i].L = psamples[i].R = sum * 32767.0f;
			}
			else
			{
				psamples[i].L = max(min(inputs[(inpos + ioffset) & filtermask].L * sum, threshold), -threshold) * outgain;
				psamples[i].R = max(min(inputs[(inpos + ioffset) & filtermask].R * sum, threshold), -threshold) * outgain;
			}

			inputs[inpos] = in;

			inpos++;
			inpos &= filtermask;

			mingain = min(mingain, sum);

			float absin = max(fabs(in.L), fabs(in.R));

			if (absin > threshold)
			{
				float g = threshold / absin;
				if (g < gain)
				{
					gain = g;
					holdcount = filtersize;
				}
			}

			fi[fipos & filtermask] = gain;
			fipos++;


			if (holdcount > 0)
			{
				if (--holdcount == 0)
				{
					rstep = (1.0f - gain) / rtime;
					rstepexp = powf(1.0f / gain, 1.0f / rtime);
				}

			}
			else
			{
				if (gain < 1.0f)
				{
					if (releaseshape == 0)
						gain += rstep;
					else
						gain *= rstepexp;

					if (gain > 1.0f)
						gain = 1.0f;
				}
			}
			

		}
	}

	void Idle()
	{
		mingain = 1.0f;
	}


private:
	float ceiling;
	float threshold;
	float gain;
	int holdcount;
	float rstep;
	float rstepexp;
	int rtime;
	int fipos;
	int inpos;
	int filtersize;
	int filtermask;
	int filtertype;
	int lookahead;
	int releaseshape;
	float *pfilter;
	float mingain;
	float fi[LIMITER_FILTER_SIZE];
	sample inputs[LIMITER_FILTER_SIZE];

};

