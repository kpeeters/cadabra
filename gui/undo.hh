
#ifndef undo_hh__
#define undo_hh__

class UndoElement {
	public:
		enum type_t { 
			 add_cell,
			 del_cell,
			 add_text,
			 del_text,
			 cut,
			 paste
		};
		DataCell 
};

typedef UndoStack std::stack<UndoElement>;

#endif
