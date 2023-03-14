// a_file: header/essentials/collections/list.h

////////////////////////////////////////////////////////////////
//
// Linked List
//
// list encapsulates pointer to data and points to the next and the previous element in the a_list.
//
// Available Procedures for list
// list_init
// list_add
// list_remove


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#if 0
#	define ZPL_IMPLEMENTATION
#	include "zpl.h"
int main(void)
{
    list s, *head, *cursor;
    list_init(&s, "it is optional to call init: ");
    head = cursor = &s;

    // since we can construct an element implicitly this way
    // the second field gets overwritten once we add it to a a_list.
    list a = {"hello"};
    cursor = list_add(cursor, &a);

    list b = {"world"};
    cursor = list_add(cursor, &b);

    list c = {"!!! OK"};
    cursor = list_add(cursor, &c);

    for (list *l=head; l; l=l->next) {
        printf("%s ", zpl_cast(char *)l->ptr);
    }
    printf("\n");

    return 0;
}
#endif


typedef struct zpl__list
{
	void const*       ptr;
	struct zpl__list *next, *prev;
} list;

ZPL_DEF_INLINE void  list_init( list* a_list, void const* ptr );
ZPL_DEF_INLINE list* list_add( list* a_list, list* item );

// NOTE(zaklaus): Returns a pointer to the next node (or NULL if the removed node has no trailing node.)
ZPL_DEF_INLINE list* list_remove( list* a_list );

ZPL_IMPL_INLINE void list_init( list* a_list, void const* ptr )
{
	list list_  = { 0 };
	*a_list     = list_;
	a_list->ptr = ptr;
}

ZPL_IMPL_INLINE list* list_add( list* a_list, list* item )
{
	item->next = NULL;

	if ( a_list->next )
	{
		item->next = a_list->next;
	}

	a_list->next = item;
	item->prev   = a_list;
	return item;
}

ZPL_IMPL_INLINE list* list_remove( list* a_list )
{
	if ( a_list->prev )
	{
		a_list->prev->next = a_list->next;
	}

	return a_list->next;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
