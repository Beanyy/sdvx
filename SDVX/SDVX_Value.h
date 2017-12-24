#ifndef SDVX_Value_h
#define SDVX_Value_h

class SDVX_Value{
	public:
		uint8_t *m_value;
		uint8_t m_min;
		uint8_t m_max;
		uint8_t m_wrap;
		int8_t m_increment;

	SDVX_Value(){}

		SDVX_Value(uint8_t *value, uint8_t min=0, uint8_t max=0, int8_t increment=1):
		 m_value(value),
		 m_min(min),
		 m_max(max),
		 m_wrap(min==max),
		 m_increment(increment) {}

		void update(int n) {
			if (!m_value || !n) return;

			int newValue = *m_value + (n*m_increment);
			if (!m_wrap && newValue > m_max)
				*m_value = m_max;
			else if (!m_wrap && newValue < m_min)
				*m_value = m_min;
			else
				*m_value = newValue;
		}
};

#endif