// file: header/essentials/collections/list.h

////////////////////////////////////////////////////////////////
//
// Linked List
//
// zpl_list encapsulates pointer to data and points to the next and the previous element in the list.
//
// Available Procedures for zpl_list
// zpl_list_init
// zpl_list_add
// zpl_list_remove


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if 0
#	define ZPL_IMPLEMENTATION
#	include "zpl.h"
int main(void)
{
    List s, *head, *cursor;
    list_init(&s, "it is optional to call init: ");
    head = cursor = &s;

    // since we can construct an element implicitly this way
    // the second field gets overwritten once we add it to a list.
    List a = {"hello"};
    cursor = list_add(cursor, &a);

    List b = {"world"};
    cursor = list_add(cursor, &b);

    List c = {"!!! OK"};
    cursor = list_add(cursor, &c);

    for (List *l=head; l; l=l->next) {
        printf("%s ", zpl_cast(char *)l->ptr);
    }
    printf("\n");

    return 0;
}
#endif


typedef struct _list
{
	void const*   ptr;
	struct _list *next, *prev;
} List;

ZPL_DEF_INLINE void  list_init( List* list, void const* ptr );
ZPL_DEF_INLINE List* list_add( List* list, List* item );

// NOTE(zaklaus): Returns a pointer to the next node (or NULL if the removed node has no trailing node.)
ZPL_DEF_INLINE List* list_remove( List* list );

ZPL_IMPL_INLINE void list_init( List* list, void const* ptr )
{
	List list_ = { 0 };
	*list      = list_;
	list->ptr  = ptr;
}

ZPL_IMPL_INLINE List* list_add( List* list, List* item )
{
	item->next = NULL;

	if ( list->next )
	{
		item->next = list->next;
	}

	list->next = item;
	item->prev = list;
	return item;
}

ZPL_IMPL_INLINE List* list_remove( List* list )
{
	if ( list->prev )
	{
		list->prev->next = list->next;
	}

	return list->next;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
