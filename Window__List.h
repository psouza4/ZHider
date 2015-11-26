struct handle_list_tag
{
	HWND hWindow;
	struct handle_list_tag *next;
};

extern struct handle_list_tag *list_of_handles;
