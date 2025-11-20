/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * Custom Redux Selector Hook
 * Typed selector hook for use throughout the application
 */

import { useSelector } from 'react-redux';
import type { RootState } from '../store/store';

export const useAppSelector = <TSelected>(
  selector: (state: RootState) => TSelected
): TSelected => {
  return useSelector(selector);
};
