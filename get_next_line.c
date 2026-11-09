/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aahmamou <aahmamou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/07 21:11:39 by aahmamou          #+#    #+#             */
/*   Updated: 2026/11/09 21:24:47 by aahmamou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static char	*append_chunk(char *stash, const char *chunk)
{
	char	*joined;

	joined = ft_strjoin(stash, chunk);
	if (!joined)
	{
		free(stash);
		return (NULL);
	}
	free(stash);
	return (joined);
}

static char	*read_chunks(int fd, char *stash, char *buffer)
{
	ssize_t	bytes;

	bytes = read(fd, buffer, BUFFER_SIZE);
	while (bytes > 0)
	{
		buffer[bytes] = '\0';
		stash = append_chunk(stash, buffer);
		if (!stash || has_newline(buffer))
			return (stash);
		bytes = read(fd, buffer, BUFFER_SIZE);
	}
	if (bytes < 0)
	{
		free(stash);
		return (NULL);
	}
	return (stash);
}

static char	*fill_stash(int fd, char *stash)
{
	char	*buffer;

	if (has_newline(stash))
		return (stash);
	buffer = malloc(sizeof(char) * ((size_t)BUFFER_SIZE + 1));
	if (!buffer)
	{
		free(stash);
		return (NULL);
	}
	stash = read_chunks(fd, stash, buffer);
	free(buffer);
	return (stash);
}

char	*get_next_line(int fd)
{
	static char	*stash;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	stash = fill_stash(fd, stash);
	if (!stash)
		return (NULL);
	line = extract_line(stash);
	if (!line)
	{
		free(stash);
		stash = NULL;
		return (NULL);
	}
	stash = trim_stash(stash);
	return (line);
}
