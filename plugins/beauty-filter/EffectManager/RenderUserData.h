#ifndef RENDER_USER_DATA_H
#define RENDER_USER_DATA_H


typedef enum {
	render_user_data_unknown = 0,
	render_user_data_loadresource,
} render_user_data_type;

class RenderUserDataBase {
public:
	virtual render_user_data_type dataType() {
		return render_user_data_unknown;
	}
};

class RenderLoadResource : public RenderUserDataBase {
public:
	render_user_data_type dataType() override {
		return render_user_data_loadresource;
	}

	bool isLoadResource() {
		return m_needLoadResource;
	}

private:
	bool m_needLoadResource = true;
};


#endif