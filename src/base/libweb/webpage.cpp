#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <memory.h>

#include "webpage.h"
#include "webgeneral.h"


#define TNOM 100
#define MAX_KEY_LEN 5

bool webpage::m_isSupportWap10 = true;

enum 
{
    TEMP_HTML,      
    TEMP_VAR,   
    TEMP_BEGINBLOCK,
    TEMP_ENDBLOCK
};

int cmp_block(const void *a, const void *b)
{
    return(strcmp(((tpl_ptr_t *)a)->name, ((tpl_ptr_t *)b)->name));
}

int cmp_block_n(const void *a, const void *b)
{
    return(strcmp((char *)a, ((tpl_ptr_t *)b)->name));
}

webpage:: webpage()
{
    bzero(&m_buf, sizeof(buffer_t));
    bzero(&m_vartable, sizeof(quick_table_t));
    bzero(&m_blocktable, sizeof(quick_table_t));
    m_head = NULL;
    m_stream = stdout;
}

webpage::~webpage()
{
    release();
}

int webpage::load(string filename)
{
    FILE        *fp;

    if((fp = fopen(filename.c_str(), "r")) == NULL)
        return -1;

    release();

    fseek(fp, 0, SEEK_END);
    m_buf.length = ftell(fp);
    rewind(fp);
    m_buf.pch = (char *)malloc(sizeof(char) * m_buf.length + MAX_KEY_LEN);
    assert(NULL != m_buf.pch);

    fread(m_buf.pch, 1, m_buf.length, fp);
    fclose(fp);
    m_buf.pch[m_buf.length] = 0;

    parse();
    
    return 0;
}

int webpage::load(const char * pBuf, int length)
{
    if(NULL == pBuf)
        return -1;

    release();

    m_buf.pch = (char *)malloc(sizeof(char) * length + MAX_KEY_LEN);
    assert(NULL != m_buf.pch);

    memcpy(m_buf.pch, pBuf, length);
    m_buf.length = length;

    parse();
    
    return 0;
}

void webpage::parse()
{
    m_head = (tpl_block_t *)malloc(sizeof(tpl_block_t));
    assert(NULL != m_head);

    memset(m_head, 0, sizeof(tpl_block_t));

    tpl_block_t *pst = m_head;

    int pos = 0, offset = 0;
    while(pos < m_buf.length)
    {
        if(m_buf.pch[pos] == '#' && m_buf.pch[pos + 1] == '#')
        {
            pst->suiv = (tpl_block_t *)malloc(sizeof(tpl_block_t));
            assert(NULL != pst->suiv);

            bzero(pst->suiv, sizeof(tpl_block_t));
            
            pst->suiv->prec = pst;
            pst->data = m_buf.pch + offset;
            pst->ndata = pos - offset;
            pst->type = TEMP_HTML;
            pst = pst->suiv;
            offset = pos;

            pos += 2;
            int namelen = 0;
            char name[TNOM + 1] = {0};
            while(m_buf.pch[pos + namelen] != '\0' && m_buf.pch[pos + namelen] != '#' && namelen < TNOM)
                namelen++;

            memcpy(name, m_buf.pch + pos, namelen);
            name[namelen] = '\0';

            pst->suiv = (tpl_block_t *)malloc(sizeof(tpl_block_t));
            assert(NULL != pst->suiv);

            memset(pst->suiv, 0, sizeof(tpl_block_t));
            
            pst->suiv->prec = pst;
            pst->type = TEMP_VAR;
            pst->name = strdup(name);
            
            assert(NULL != pst->name);

            m_vartable.pst = (tpl_ptr_t *)realloc(m_vartable.pst, sizeof(tpl_ptr_t) * (m_vartable.nlength + 1));
            assert(NULL != m_vartable.pst);
            m_vartable.pst[m_vartable.nlength].name = strdup(name);
            assert(NULL != m_vartable.pst[m_vartable.nlength].name);
            m_vartable.pst[m_vartable.nlength].block = pst;
            m_vartable.nlength++;

            pst = pst->suiv;
            pos += namelen + 2;
            offset = pos;
        }
        else if(m_buf.pch[pos] == '<' && m_buf.pch[pos + 1] == '!' 
            && m_buf.pch[pos + 2] == '-' && m_buf.pch[pos + 3] == '-')
        {
            pst->suiv = (tpl_block_t *)malloc(sizeof(tpl_block_t));
            assert(NULL != pst->suiv);

            memset(pst->suiv, 0, sizeof(tpl_block_t));
            pst->suiv->prec = pst;
            pst->data = m_buf.pch + offset;
            pst->ndata = pos - offset;
            pst->type = TEMP_HTML;
            pst = pst->suiv;
            offset = pos;

            pos += 4;   
            
            while (isspace(m_buf.pch[pos]))
                pos ++;

            int nlen = 0;
            char token[TNOM + 1] = {0};
            while (m_buf.pch[pos] != '-' && m_buf.pch[pos] != '>'
                && !isspace(m_buf.pch[pos]) && nlen < TNOM)
            {
                token[nlen++] = m_buf.pch[pos];
                pos++;
            }
            
            token[nlen] = '\0';
            
            while (isspace(m_buf.pch[pos]))
                pos++;
            
            nlen = 0;
            char name[TNOM + 1] = {0};
            while (m_buf.pch[pos] != '-' && m_buf.pch[pos] != '>'
                && !isspace(m_buf.pch[pos]) && nlen < TNOM)
            {
                name[nlen++] = m_buf.pch[pos];
                pos++;
            }
            
            name[nlen] = '\0';

            while (!(m_buf.pch[pos] == '>' || m_buf.pch[pos] == '#'))
                pos++;

            int type = -1;

            if (strcmp(token, "#BeginBlock") == 0)
                type = TEMP_BEGINBLOCK;
            else if(strcmp(token, "#EndBlock") == 0)
                type = TEMP_ENDBLOCK;

            if (type != -1)
            {
                pst->suiv = (tpl_block_t *)malloc(sizeof(tpl_block_t));
                assert(NULL != pst->suiv);
                bzero(pst->suiv, sizeof(tpl_block_t));
                pst->type = type;
                pst->suiv->prec = pst;
                pst->name = strdup(name);

                assert(NULL != pst->name);

                if(TEMP_BEGINBLOCK == type)
                {
                    m_blocktable.pst = (tpl_ptr_t *)realloc(m_blocktable.pst,
                            sizeof(tpl_ptr_t) * (m_blocktable.nlength + 1));

                    assert(NULL != m_blocktable.pst);

                    m_blocktable.pst[m_blocktable.nlength].name = strdup(name);

                    assert(NULL != m_blocktable.pst[m_blocktable.nlength].name);

                    m_blocktable.pst[m_blocktable.nlength].block = pst;
                    m_blocktable.nlength++;
                }
          
                pst = pst->suiv;
            }
            else
            {
                /* the whole block is taken for html */
                pst->suiv = (tpl_block_t *)malloc(sizeof(tpl_block_t));
                assert(NULL != pst->suiv);
                bzero(pst->suiv, sizeof(tpl_block_t));
                pst->suiv->prec = pst;
                pst->data = m_buf.pch + offset;
                pst->ndata = pos - offset + 1;
                pst->type = TEMP_HTML;
                pst = pst->suiv;                
            }

            offset = pos + 1;
        }

        pos++;
    }

    if (offset < m_buf.length)
    {
        pst->suiv = NULL;
        pst->data = m_buf.pch + offset;
        pst->ndata = m_buf.length - offset;
        pst->type = TEMP_HTML;
    }

    qsort(m_vartable.pst, m_vartable.nlength, sizeof(tpl_ptr_t), cmp_block);
    qsort(m_blocktable.pst, m_blocktable.nlength, sizeof(tpl_ptr_t), cmp_block);
}

void webpage::output(FILE *stream)
{
    m_stream = stream;
    output();
}

void webpage::output()
{
    tpl_block_t         *pst = m_head;
    char                *ptr = NULL; 

    string wap20;
    while(pst != NULL)
    {
        if(pst->type == TEMP_BEGINBLOCK)
        {
            char *pch = strdup(pst->name);

            assert(NULL != pch);

            pst = pst->suiv;

            while (pst && !(pst->type == TEMP_ENDBLOCK && strcmp(pst->name, pch) == 0))             
                pst = pst->suiv;

            if(pch != NULL)
            {
                free(pch);
                pch = NULL;
            }
        }
        
        if(pst && (pst->type == TEMP_HTML || pst->type == TEMP_VAR))
        {
            if(NULL != pst->data && pst->ndata != 0)
            {
                ptr = (char *)malloc(pst->ndata + 1);
                assert(NULL != ptr);
                memcpy(ptr, pst->data, pst->ndata);
                ptr[pst->ndata] = '\0';

                    if(m_isSupportWap10)
                    {
                        FCGX_PutStr(ptr, pst->ndata, fcgi_out);
                    }

                    else
                    {
                        wap20 += ptr;
                    }

                    free(ptr);
            }
        }

        if(pst == NULL)
            break;
        
        pst = pst->suiv;
    }
#ifdef WEBLIB_WITH_FASTCGI
    FCGX_FFlush(fcgi_out);
#else
    fflush(m_stream);
#endif

}


int webpage::output(string filename)
{
    FILE *fp;
    if((fp = fopen(filename.c_str(), "w")) == NULL)
        return -1;

    output(fp);
    return 0;
}

tpl_ptr_t * webpage::find(const char * elem, 
                        const quick_table_t *base,
                        int(* cmpfunc)(const void *, const void *), 
                        int * nb)
{
    void        *ptr = NULL;
    int         pos = 0;
    
    *nb = 0;

    ptr = bsearch((const void *)elem, (void *)base->pst, base->nlength, sizeof(tpl_ptr_t), cmpfunc);
    if (!ptr)
        return NULL;

    pos = ((char *)ptr - (char *)base->pst) / sizeof(tpl_ptr_t);

    while(pos > 0 && cmpfunc(elem, (const void *)((char *)base->pst + (pos - 1 ) * sizeof(tpl_ptr_t))) == 0)
        pos--;

    while(pos + *nb < base->nlength && cmpfunc(elem, (const void *)((char *)base->pst + (pos + *nb) * sizeof(tpl_ptr_t))) == 0)
        (*nb)++;

    return((tpl_ptr_t *)(((char *)base->pst) + pos * sizeof(tpl_ptr_t)));
}

void webpage::set(string token, string val)
{
    tpl_block_t         *pst;
    tpl_ptr_t           *result;
    int i, nb;

    result = find(token.c_str(), &m_vartable, cmp_block_n, &nb);
    if (result)
    {
        for (i = 0; i < nb; i++)
        {
            pst = result[i].block;
            
            if(pst->data != NULL)
                free(pst->data);
            
            pst->data = strdup(val.c_str());

            assert(NULL != pst->data);

            pst->ndata = val.length();
        }
    }
}

void webpage::set(string token, int val)
{
    set(token, strFormat("%d",val));
}

void webpage::set(string token, unsigned int val)
{
    set(token, strFormat("%u",val));
}

void webpage::set(string token, unsigned long long val)
{
    set(token, strFormat("%llu",val));
}

void webpage::gen_block(tpl_block_t * curr, tpl_block_t * suiv)
{
    tpl_block_t *pst;

    pst = (tpl_block_t *)malloc(sizeof(tpl_block_t));
    assert(NULL != pst);
    memset(pst, 0, sizeof(tpl_block_t));

    if(curr->ndata != 0)
    {
        pst->data = (char *)malloc(curr->ndata);
        assert(NULL != pst->data);
        memcpy(pst->data, curr->data, curr->ndata);
        pst->ndata = curr->ndata;
        pst->alloc = 1;
        pst->type = TEMP_HTML;

        pst->prec = suiv->prec;
        if(suiv->prec)
            suiv->prec->suiv = pst;
        pst->suiv = suiv;
        suiv->prec = pst;
    }
}

void webpage::del_block(tpl_block_t * &pst)
{
    pst->prec->suiv = pst->suiv;
    pst->suiv->prec = pst->prec;

    tpl_block_t * tmp = pst->prec;
    free(pst);

    pst = tmp;
}

void webpage::set(string block)
{
    tpl_block_t     *pst, *begin;
    int             i, nb;
    tpl_ptr_t       *result;


    result = find(block.c_str(), &m_blocktable, cmp_block_n, &nb);
    if(result)
    {
        for (i = 0; i < nb; i++) 
        {
            pst = begin = result[i].block;
            pst = pst->suiv;

            //  we go through all the blocks till the corresponding ENDTABLE
            while (pst && !(pst->type == TEMP_ENDBLOCK && strcmp(pst->name, block.c_str()) == 0)) 
            {
                if(pst->type == TEMP_BEGINBLOCK)
                {
                    char *pch = strdup(pst->name);

                    assert(NULL != pch);

                    pst = pst->suiv;
                    while (pst && !(pst->type == TEMP_ENDBLOCK && strcmp(pst->name, pch) == 0))
                        pst = pst->suiv;

                    if(NULL != pch)
                        free(pch);
                }
                else
                {
                    gen_block(pst, begin);

                    if(pst->alloc == 1)
                        del_block(pst);
                }

                if(pst == NULL)
                    break;

                pst = pst->suiv;
            }           
        }
    }
}

void webpage::set_bloc(string block)
{
    set(block);
}

void webpage::release()
{
    tpl_block_t *pst, *tmp;
    
    pst = m_head;
    while (pst)
    {
        if (pst->name)  
            free(pst->name);
        
        if (pst->alloc && pst->data)
            free(pst->data);
        
        tmp = pst;
        pst = pst->suiv;

        free(tmp);
    }
    
    m_head = NULL;
    
    for(int i = 0; i < m_vartable.nlength; i++)
        free(m_vartable.pst[i].name);

    for(int i = 0; i < m_blocktable.nlength; i++)
        free(m_blocktable.pst[i].name);

    if (m_vartable.pst)
    {
        free(m_vartable.pst);
        bzero(&m_vartable, sizeof(quick_table_t));
    }
    
    if (m_blocktable.pst)
    {
        free(m_blocktable.pst);
        bzero(&m_blocktable, sizeof(quick_table_t));
    }
    

    if (m_buf.pch)
    {
        free(m_buf.pch);
        bzero(&m_buf, sizeof(buffer_t));
    }
}


void webpage::setsupportwap10(bool issupportwap10)
{
    m_isSupportWap10 = issupportwap10;
}
