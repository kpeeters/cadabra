
#ifndef caslink_hh__
#define caslink_hh__

class CASLink {
	public:
		

	private:
		void received_expression(const std::string&);
		void received_texcomment(const std::string&);
		void input_ended();
};

#endif
